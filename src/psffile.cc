#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

#include <algorithm>

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

PSFFile::PSFFile(std::string filename) : 
    m_header(NULL), m_types(NULL), m_sweeps(NULL), 
    m_traces(NULL), m_sweepvalues(NULL), m_nonsweepvalues(NULL) {

    m_filename = filename;
    m_fd = -1;
}

PSFFile::~PSFFile() {
    if (m_header)
        delete (m_header);
    if (m_types)
        delete (m_types);
    if (m_sweeps)
        delete (m_sweeps);
    if (m_traces)
        delete (m_traces);
    if (m_sweepvalues)
        delete (m_sweepvalues);
    if (m_nonsweepvalues)
        delete (m_nonsweepvalues);
    close();
}

SectionMap PSFFile::load_sections(const char *buf, int size){
    std::vector<Section> sections;
    uint32_t section_offset = 4;

    int section_num = 0;
    while ( section_offset < size ){
        Section section;
        uint32_t section_type = GET_INT32(buf + section_offset);
        if ( ! (section_type == HeaderSection::type))
            break;
        section.n = section_num; 
        section.offset = section_offset;

        uint32_t section_end = GET_INT32(buf + section_offset + 4);
        section.size = section_end - section_offset;

        sections.push_back(section);

        section_num++;    
        section_offset = section_end;
    }
    if (sections.size() < 3){
        throw InvalidFileError();
    }

    m_header = new HeaderSection();
    m_header->deserialize(buf + sections[SECTION_HEADER].offset,
                          sections[SECTION_HEADER].offset);

    int num_sweep_points = 0;
    bool has_sweep = get_header_properties().hasprop("PSF sweep points");
    if (has_sweep)
        num_sweep_points = get_header_properties().find("PSF sweep points");
    
    if (num_sweep_points == 0)
        sections[2].n = SECTION_VALUE;
    
    SectionMap section_map;
    for (auto section: sections)
        section_map[section.n] = section;
    
    return section_map;
}

SectionMap PSFFile::load_table_of_contents(const char *buf, int size) {
    // Last word contains the size of the data
    uint32_t datasize;
    datasize = GET_INT32(buf + size - 4);

    int nsections = (size - datasize - 12) / 8;
    int lastoffset = 0, lastsectionnum = -1;
    const char *toc = buf + size - 12 - nsections * 8;

    SectionMap section_map;
    
    for (int i = 0; i < nsections; i++) {
        Section section;
        section.n = GET_INT32(toc + 8 * i);
        section.offset = GET_INT32(toc + 8 * i + 4);

        if (i > 0)
            section_map[lastsectionnum].size = section.offset - lastoffset;
        
        if (i == nsections - 1)
            section.size = size - section.offset;

        section_map[section.n] = section;

        lastoffset = section.offset;
        lastsectionnum = section.n;
    }

    m_header = new HeaderSection();
    m_header->deserialize(buf + section_map[SECTION_HEADER].offset,
                          section_map[SECTION_HEADER].offset);

    return section_map;
}

void PSFFile::deserialize(const char *buf, int size) {
    // Read section index table
    SectionMap sections;
    if (is_done()) {
        sections = load_table_of_contents(buf, size);
    } else {
        sections = load_sections(buf, size);
    }

    // Read types
    if (sections.find(SECTION_TYPE) != sections.end()) {
        m_types = new TypeSection();
        m_types->deserialize(buf + sections[SECTION_TYPE].offset,
         sections[SECTION_TYPE].offset);
    }

    // Read sweeps
    if (sections.find(SECTION_SWEEP) != sections.end()) {
        m_sweeps = new SweepSection(this);
        m_sweeps->deserialize(buf + sections[SECTION_SWEEP].offset,
         sections[SECTION_SWEEP].offset);
    }

    // Read traces
    if (sections.find(SECTION_TRACE) != sections.end()) {
        m_traces = new TraceSection(this);
        m_traces->deserialize(buf + sections[SECTION_TRACE].offset,
         sections[SECTION_TRACE].offset);
    }

    // Read values
    if (sections.find(SECTION_VALUE) != sections.end()) {
        if (m_sweeps != NULL) {
            m_sweepvalues = new ValueSectionSweep(this);
            m_sweepvalues->deserialize(buf + sections[SECTION_VALUE].offset,
                                       sections[SECTION_VALUE].offset);
        } else {
            m_nonsweepvalues = new ValueSectionNonSweep(this);
            m_nonsweepvalues->deserialize(buf + sections[SECTION_VALUE].offset,
                                          sections[SECTION_VALUE].offset);
        }
    }
}

void PSFFile::open() {
    m_fd = ::open(m_filename.c_str(), O_RDONLY);
  
    if (m_fd == -1)
	throw FileOpenError();
  
    m_size = lseek(m_fd, 0, SEEK_END);
  
    m_buffer = (char *)mmap(0, m_size, PROT_READ, MAP_SHARED, m_fd, 0);

	deserialize((const char *)m_buffer, m_size);
}

void PSFFile::close() {
    munmap((void*) m_buffer, m_size);
    
    if(m_fd != -1) {
	int rval = ::close(m_fd);

	if (rval == -1)
	    throw FileCloseError();

	m_fd = -1;
    }
}

bool PSFFile::is_done() const {
    std::ifstream fstr(m_filename.c_str());
	
    fstr.seekg(-12, std::ios::end);

    char clarissa[9];
	
    fstr.read(clarissa, 8);
    clarissa[8]=0;
	
    return !strcmp(clarissa, "Clarissa");
}

NameList PSFFile::get_param_names() const {
    if (m_sweeps != NULL)
	return m_sweeps->get_names();
    else
	return NameList();
}

PSFVector* PSFFile::get_param_values() const {
    if (m_sweepvalues != NULL) 
	return m_sweepvalues->get_param_values();
    else
	return NULL;
}

PSFVector* PSFFile::get_values(std::string name) const {
    if(m_sweepvalues)
	return m_sweepvalues->get_values(name);
    else
	return NULL;
}	

const PropertyBlock &PSFFile::get_value_properties(std::string name) const {
  //FIXME, check for NULL m_nonsweepvalues
  return m_nonsweepvalues->get_value_properties(name);
}

const PSFScalar& PSFFile::get_value(std::string name) const {
    if(m_nonsweepvalues)
	return m_nonsweepvalues->get_value(name);
}

NameList PSFFile::get_names() const {
    if(m_traces) {
	return m_traces->get_names();
    } else if(m_nonsweepvalues) {
	return m_nonsweepvalues->get_names();
    }
}
