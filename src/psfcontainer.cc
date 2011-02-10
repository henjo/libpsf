#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

//
// Container
//

Container::~Container() {
    for(int i=0; i < size(); i++)
	delete(at(i));
}

Chunk * Container::deserialize_child(const char **buf) {
    Chunk *child = NULL;

    int childtype = GET_INT32(*buf);
	
    child = child_factory(childtype);

    if(child)
	(*buf) += child->deserialize(*buf);
    else 
	// If child is NULL an endmarker was found and its chunk type shall be consumed
	(*buf) += 4;

    return child;
}

Chunk & Container::get_child(int id) {
    for(const_iterator child=begin(); child != end(); child++) {
	if(id == (*child)->get_id())
	    return **child;
    }	
}
Chunk & Container::get_child(std::string name) {
    for(const_iterator child=begin(); child != end(); child++) {
	if(name == (*child)->get_name())
	    return **child;
    }	
}

NameList Container::get_names() {
    NameList result;

    for(const_iterator i=begin(); i != end(); i++)
	result.push_back((*i)->get_name());

    return result;
}

//
// SimpleContainer
//

int SimpleContainer::deserialize(const char *buf, int abspos) {
    const char *startbuf = buf;

    buf += Chunk::deserialize(buf);
	
    uint32_t endpos = GET_INT32(buf);
    buf += sizeof(uint32_t);

    int i = 0;
    while(abspos + (buf-startbuf) < endpos) {
	Chunk *chunk = deserialize_child(&buf); 
	if(chunk)
	    add_child(chunk);
	else	
	    break;
    }

    return buf - startbuf;
}

void SimpleContainer::print(std::ostream &stream) {
    stream << "SimpleContainer(";
    for(Container::const_iterator child=begin(); child !=end(); child++) 
	stream << **child << " " ;
    stream << ")";	
};

