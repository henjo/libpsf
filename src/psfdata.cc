#include <iostream>
#include <fstream>
#include <string.h>
#include <stdint.h>
#include <endian.h>
#include <list>
#include <map>
#include <algorithm>

#include "psf.h"
#include "psfinternal.h"


std::ostream &operator<<(std::ostream &stream, PSFScalar &o)
{
    o.print(stream);
    return stream; // must return stream
}
 
// PSFInt8Scalar
template<>
int PSFInt8Scalar::deserialize(const char *buf) {
    value = *((int8_t *)buf+3);
    return 4;
}
template<>
PSFInt8Scalar::operator int() const {
    return value;
};

// PSFInt32Scalar
template<>
int PSFInt32Scalar::deserialize(const char *buf) {
    value = GET_INT32(buf);
    return 4;
}
template<>
PSFInt32Scalar::operator int() const {
    return value;
};
template<>
PSFInt32Scalar::operator double() const {
    return value;
};


// PSFDoubleScalar
template<>
int PSFDoubleScalar::deserialize(const char *buf) {
    *((uint64_t *)&value) = be64toh(*((uint64_t *)buf));
    return 8;
}
template<>
PSFDoubleScalar::operator int() const {
    return (int)value;
};
template<>
PSFDoubleScalar::operator double() const {
    return value;
};

// PSFDoubleScalar
template<>
int PSFComplexDoubleScalar::deserialize(const char *buf) {
    *((uint64_t *)&value) = be64toh(*((uint64_t *)buf));
    return 8;
}
template<>
PSFComplexDoubleScalar::operator int() const {
    throw NotImplemented();
};
template<>
PSFComplexDoubleScalar::operator double() const {
    throw NotImplemented();
};

// PSFStringScalar
template<>
int PSFStringScalar::deserialize(const char *buf) {
    int len = GET_INT32(buf);

    value = std::string(buf+4, len);

    // Align to 32-bit boundary
    return 4 + len + ((4-len) & 3);    
};
template<>
PSFStringScalar::operator int() const {
    return atoi(value.c_str());
};
template<>
PSFStringScalar::operator double() const {
    return atof(value.c_str());
};

// PSFStructScalar
template<>
int StructScalar::deserialize(const char *buf) {
    return value.deserialize(buf);
}

// Struct
Struct::~Struct() {
    for(Struct::const_iterator i=begin(); i != end(); i++)
	delete(i->second);
}
int Struct::deserialize(const char *buf) {
    const char *startbuf = buf;

    for(ChildList::const_iterator itemdef=structdef->begin(); 
	itemdef != structdef->end(); itemdef++) {

	PSFScalar *itemvalue = ((DataTypeDef *)(*itemdef))->new_scalar();

	buf += itemvalue->deserialize(buf);
	
	Struct &me = *this;
	std::string key((*itemdef)->get_name());
	me[key] = itemvalue;
    }
    
    return buf - startbuf;
}
template<>
StructScalar::operator int() const {
    return -1;
};
template<>
StructScalar::operator double() const {
    return -1;
};

template<>
PSFDoubleVector::PSFVectorT() { init = 0; }
template<>
PSFInt32Vector::PSFVectorT() { init = 0; }
template<>
PSFComplexDoubleVector::PSFVectorT() { init = PSFComplexDouble(0,0); }


std::ostream &operator<<(std::ostream &stream, Struct &o) {
    stream << "Struct(";
    for(Struct::iterator i=o.begin(); i != o.end(); i++)
	stream << "(" << i->first << "," << *(i->second) << ")";
    stream << ")";
}

int Struct::datasize() const {
    return structdef->datasize(); 
}

int psfdata_size(int datatypeid) {
    switch(datatypeid) {
    case TYPEID_INT8:
	return 4;
    case TYPEID_INT32:
	return 4;
    case TYPEID_DOUBLE:
	return 8;
    case TYPEID_COMPLEXDOUBLE:
	return 16;
    default:
	throw UnknownType(datatypeid);    
    }
}
