#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

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
    if(m_header)
	delete(m_header);
    if(m_types)
	delete(m_types);
    if(m_sweeps)
	delete(m_sweeps);
    if(m_traces)
	delete(m_traces);
    if(m_sweepvalues)
	delete(m_sweepvalues);
    if(m_nonsweepvalues)
	delete(m_nonsweepvalues);
    close();
}

void PSFFile::deserialize(const char *buf, int size) {
    // Last word contains the size of the data
    uint32_t datasize;	
    datasize = GET_INT32(buf+size-4);
	
    // Read section index table

    std::map<int, Section> sections;

    int nsections = (size - datasize - 12) / 8;
    int lastoffset = 0, lastsectionnum = -1;

    const char *toc = buf + size - 12 - nsections*8;
    Section section;
    for(int i=0; i < nsections; i++) {
	section.n = GET_INT32(toc + 8*i);
	section.offset = GET_INT32(toc + 8*i + 4);

	if (i>0)
	    sections[lastsectionnum].size = section.offset - lastoffset;

	sections[section.n] = section;

	lastoffset = section.offset;
	lastsectionnum = section.n;
    }
    sections[section.n].size = size - section.offset;

    m_header = new HeaderSection();
    m_header->deserialize(buf + sections[SECTION_HEADER].offset, sections[SECTION_HEADER].offset);

    // Read types
    if (sections.find(SECTION_TYPE) != sections.end()) {
	m_types = new TypeSection();
	m_types->deserialize(buf + sections[SECTION_TYPE].offset, sections[SECTION_TYPE].offset);
    }

    // Read sweeps
    if (sections.find(SECTION_SWEEP) != sections.end()) {	
	m_sweeps = new SweepSection(this);
	m_sweeps->deserialize(buf + sections[SECTION_SWEEP].offset, sections[SECTION_SWEEP].offset);
    }

    // Read traces
    if (sections.find(SECTION_TRACE) != sections.end()) {	
	m_traces = new TraceSection(this);
	m_traces->deserialize(buf + sections[SECTION_TRACE].offset, sections[SECTION_TRACE].offset);
    }

    // Read values
    if (sections.find(SECTION_VALUE) != sections.end()) {	
	if(m_sweeps != NULL) {
	    m_sweepvalues = new ValueSectionSweep(this);
	    m_sweepvalues->deserialize(buf + sections[SECTION_VALUE].offset, sections[SECTION_VALUE].offset);
	} else {
	    m_nonsweepvalues = new ValueSectionNonSweep(this);
	    m_nonsweepvalues->deserialize(buf + sections[SECTION_VALUE].offset, sections[SECTION_VALUE].offset);
	}
    }

}

void PSFFile::open() {
    m_fd = ::open(m_filename.c_str(), O_RDONLY);
  
    if (m_fd == -1)
	throw FileOpenError();
  
    m_size = lseek(m_fd, 0, SEEK_END);
  
    m_buffer = (char *)mmap(0, m_size, PROT_READ, MAP_SHARED, m_fd, 0);
  
    if(validate())
	deserialize((const char *)m_buffer, m_size);
    else
	throw InvalidFileError();
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

bool PSFFile::validate() const {
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

PropertyMap PSFFile::get_value_properties(std::string name) const {
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
