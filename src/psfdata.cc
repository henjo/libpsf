using namespace std;

#include <iostream>
#include <fstream>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <endian.h>
#include <list>
#include <map>
#include <algorithm>

#include "psf.h"
#include "psfinternal.h"


ostream &operator<<(ostream &stream, PSFData &o)
{
    o.print(stream);
    return stream; // must return stream
}
 

int Int8::deserialize(const char *buf) {
    value = *((int8_t *)buf+3);
    return 4;
}

int Int32::deserialize(const char *buf) {
    value = GET_INT32(buf);
    return 4;
}

int UInt32::deserialize(const char *buf) {
    value = GET_INT32(buf);
    return 4;
}

int Float64::deserialize(const char *buf) {
    *((uint64_t *)&value) = be64toh(*((uint64_t *)buf));
    return 8;
}

int String::deserialize(const char *buf) {
    int len = GET_INT32(buf);

    value = string(buf+4, len);

    // Align to 32-bit boundary
    return 4 + len + ((4-len) & 3);    
};

int Struct::deserialize(const char *buf) {
    const char *startbuf = buf;

    for(ChildList::const_iterator itemdef=structdef->begin(); 
	itemdef != structdef->end(); itemdef++) {

	PSFData *itemvalue = ((DataTypeDef *)(*itemdef))->get_data_object();

	buf += itemvalue->deserialize(buf);
	
	Struct &me = *this;
	string key((*itemdef)->get_name());
	me[key] = itemvalue;
    }
    
    return buf - startbuf;
}

void Struct::print(ostream &stream) {
    stream << "Struct(";
    for(Struct::iterator i=begin(); i != end(); i++)
	stream << "(" << i->first << "," << *(i->second) << ")";
    stream << ")";
}

int Struct::datasize() const {
    return structdef->datasize(); 
}

PSFDataVector *PSFDataVector::create(int type_id) {	
    switch(type_id) {
    case Float64::type_id:
	return new Float64Vector();
    case Struct::type_id:
	return new StructVector();
    default:
	throw UnknownType(type_id);    
    }
    
}

PSFData *psfdata_from_typeid(int type_id) {	
    switch(type_id) {
    case String::type_id:
	return new String();
    case Int8::type_id:
	return new Int8();
    case Int32::type_id:
	return new Int32();
    case Float64::type_id:
	return new Float64();
    default:
	throw UnknownType(type_id);    
    }
}

int psfdata_size(int type_id) {
    PSFData *tmp = psfdata_from_typeid(type_id);

    int size = tmp->datasize();
    delete tmp;

    return size;
}
