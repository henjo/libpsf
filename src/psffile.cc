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

void PSFFile::deserialize(const char *buf, int size) {
    // Last word contains the size of the data
    uint32_t datasize;	
    datasize = GET_INT32(buf+size-4);
	
    // Read section index table

    std::map<int, Section> sections;

    int nsections = (size - datasize - 12) / 8;
    int lastoffset = 0, lastsectionnum = -1;
    Section section;
    for(int i=0; i < nsections; i++) {
	section.n = GET_INT32(buf + datasize + 8*i);
	section.offset = GET_INT32(buf + datasize + 8*i + 4);

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

    off_t size = lseek(fd, 0, SEEK_END);
    
    if (fd == -1)
	throw FileOpenError();

    const char *buffer = (const char *)mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);

    if(validate())
	deserialize(buffer, size);
}

void PSFFile::close() {
    if(fd != -1) {
	::close(fd);
	fd = -1;
    }
}

bool PSFFile::validate() {
    std::ifstream fstr(filename.c_str());
	
    fstr.seekg(-12, std::ios::end);

    char clarissa[9];
	
    fstr.read(clarissa, 8);
    clarissa[8]=0;
	
    return !strcmp(clarissa, "Clarissa");
}	

PSFVector * PSFFile::get_param_values() {
    return sweepvalues->get_param_values();
}

PSFVector * PSFFile::get_values(std::string name) {
    if(sweepvalues)
	return sweepvalues->get_values(name);
}	

PSFScalar * PSFFile::get_value(std::string name) {
    if(nonsweepvalues)
	return nonsweepvalues->get_value(name);
}

NameList PSFFile::get_names() {
    if(traces) {
	return traces->get_names();
    } else if(nonsweepvalues) {
	return nonsweepvalues->get_names();
    }
}
