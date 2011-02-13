#include <assert.h>

#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

int Index::deserialize(const char *buf) {
    const char *startbuf = buf;

    buf += Chunk::deserialize(buf);
	
    uint32_t size = GET_INT32(buf);
    buf += sizeof(uint32_t);

    int id, offset;
    for(int i=0; i < size; i+=8) {
	id = GET_INT32(buf+i);
	offset = GET_INT32(buf+i+4);
    }
	
    return buf + size - startbuf;
}


int TraceIndex::deserialize(const char *buf) {
    const char *startbuf = buf;

    buf += Chunk::deserialize(buf);
	
    uint32_t size = GET_INT32(buf);
    buf += sizeof(uint32_t);

    int id, offset, extra1, extra2;
    for(int i=0; i < size; i+=16) {
	id = GET_INT32(buf+i);
	offset = GET_INT32(buf+i+4);
	extra1 = GET_INT32(buf+i+8);
	extra2 = GET_INT32(buf+i+12);
	//	std::cout << "(" << std::hex << id << "," << offset << "," << extra1 << "," << extra2 << ")" << std::endl;
    }
	
    return buf + size - startbuf;
}


int IndexedContainer::deserialize(const char *buf, int abspos) {
    const char *startbuf = buf;

    buf += Chunk::deserialize(buf);
	
    uint32_t endpos = GET_INT32(buf);
    buf += sizeof(uint32_t);


    // Sub container
    uint32_t subcontainer_typeid = GET_INT32(buf); 
    buf += sizeof(uint32_t); 
    
    assert(subcontainer_typeid == 22);

    uint32_t subendpos = GET_INT32(buf);
    buf += sizeof(uint32_t);

    int i = 0;
    while(abspos + (buf-startbuf) < subendpos) {
	Chunk *chunk = deserialize_child(&buf); 
	if(chunk)
	    add_child(chunk);
	else	
	    break;
    }

    Index index;
    buf += index.deserialize(buf);

    for(Container::const_iterator child=begin(); child != end(); child++) {
	idmap[(*child)->get_id()] = *child;
	const std::string &name = (*child)->get_name();
	namemap[name] = *child;
    }

    return endpos - abspos;
}	

void IndexedContainer::print(std::ostream &stream) const {
    stream << "IndexedContainer(";
    for(Container::const_iterator child=begin(); child !=end(); child++) 
	stream << **child << " " ;
    stream << ")";	
}

const Chunk & IndexedContainer::get_child(int id) const {
    return *idmap.find(id)->second;
}

const Chunk & IndexedContainer::get_child(std::string name) const {
    return *namemap.find(name)->second;
}

