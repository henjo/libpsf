#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

PSFFile::PSFFile(std::string _filename) : 
    header(NULL), types(NULL), sweeps(NULL), 
    traces(NULL), sweepvalues(NULL), nonsweepvalues(NULL) {

    filename = _filename;
    fd = -1;
}

PSFFile::~PSFFile() {
    if(header)
	delete(header);
    if(types)
	delete(types);
    if(sweeps)
	delete(sweeps);
    if(traces)
	delete(traces);
    if(sweepvalues)
	delete(sweepvalues);
    if(nonsweepvalues)
	delete(nonsweepvalues);
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

    header = new HeaderSection();
    header->deserialize(buf + sections[SECTION_HEADER].offset, sections[SECTION_HEADER].offset);

    // Read types
    if (sections.find(SECTION_TYPE) != sections.end()) {
	types = new TypeSection();
	types->deserialize(buf + sections[SECTION_TYPE].offset, sections[SECTION_TYPE].offset);
    }

    // Read sweeps
    if (sections.find(SECTION_SWEEP) != sections.end()) {	
	sweeps = new SweepSection(this);
	sweeps->deserialize(buf + sections[SECTION_SWEEP].offset, sections[SECTION_SWEEP].offset);
    }

    // Read traces
    if (sections.find(SECTION_TRACE) != sections.end()) {	
	traces = new TraceSection(this);
	traces->deserialize(buf + sections[SECTION_TRACE].offset, sections[SECTION_TRACE].offset);
    }

    // Read values
    if (sections.find(SECTION_VALUE) != sections.end()) {	
	if(sweeps != NULL) {
	    sweepvalues = new ValueSectionSweep(this);
	    sweepvalues->deserialize(buf + sections[SECTION_VALUE].offset, sections[SECTION_VALUE].offset);
	} else {
	    nonsweepvalues = new ValueSectionNonSweep(this);
	    nonsweepvalues->deserialize(buf + sections[SECTION_VALUE].offset, sections[SECTION_VALUE].offset);
	}
    }

}

void PSFFile::open() {
    fd = ::open(filename.c_str(), O_RDONLY);
  
    if (fd == -1)
	throw FileOpenError();
  
    size = lseek(fd, 0, SEEK_END);
  
    buffer = (char *)mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
  
    if(validate())
	deserialize((const char *)buffer, size);
    else
	throw InvalidFileError();
}

void PSFFile::close() {
    int rval;

    munmap((void*) buffer, size);
    
    if(fd != -1) {
	rval = ::close(fd);
	if (rval == -1)
	    throw FileCloseError();
	fd = -1;
    }
}

bool PSFFile::validate() const {
    std::ifstream fstr(filename.c_str());
	
    fstr.seekg(-12, std::ios::end);

    char clarissa[9];
	
    fstr.read(clarissa, 8);
    clarissa[8]=0;
	
    return !strcmp(clarissa, "Clarissa");
}	


NameList PSFFile::get_param_names() const {
    if (sweeps != NULL)
	return sweeps->get_names();
    else
	return NameList();
}

PSFVector* PSFFile::get_param_values() const {
    if (sweepvalues != NULL) 
	return sweepvalues->get_param_values();
    else
	return NULL;
}

PSFVector* PSFFile::get_values(std::string name) const {
    if(sweepvalues)
	return sweepvalues->get_values(name);
    else
	return NULL;
}	

PropertyMap PSFFile::get_value_properties(std::string name) const {
    return nonsweepvalues->get_value_properties(name);
}

const PSFScalar& PSFFile::get_value(std::string name) const {
    if(nonsweepvalues)
	return nonsweepvalues->get_value(name);
}

NameList PSFFile::get_names() const {
    if(traces) {
	return traces->get_names();
    } else if(nonsweepvalues) {
	return nonsweepvalues->get_names();
    }
}
