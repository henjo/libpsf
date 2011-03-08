#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

//
// DataTypeDef
//

DataTypeDef::~DataTypeDef() {
    if(structdef)
	delete(structdef);
}

int DataTypeDef::deserialize(const char *buf) {
    const char *startbuf = buf;

    buf += Chunk::deserialize(buf);

    id = GET_INT32(buf); buf+=4;

    buf += name.deserialize(buf);
	
    int arraytype = GET_INT32(buf); buf+=4;

    datatypeid = GET_INT32(buf);
    buf += 4;

    if(datatypeid == 16) {
	structdef = new StructDef();
	buf += structdef->deserialize(buf);
	_datasize = structdef->datasize();
    } else
	_datasize = psfdata_size(datatypeid);

    // Read optional properties
    while(true) {  	
	int chunktype = GET_INT32(buf);

	if(Property::ischunk(chunktype)) {
	    Property prop;
	    buf += prop.deserialize(buf);
	    properties.push_back(prop);
	} else
	    break;
    }

    return buf - startbuf;
};

void * DataTypeDef::new_dataobject() const {
    switch(datatypeid) {
    case TYPEID_INT8:
	return new PSFInt8;
    case TYPEID_INT32:
	return new PSFInt32;
    case TYPEID_DOUBLE:
	return new PSFDouble;
    case TYPEID_STRUCT:
	return structdef->new_dataobject();
    default:
	throw UnknownType(datatypeid);    
    }
}

int DataTypeDef::deserialize_data(void *data, const char *buf) const {
    switch(datatypeid) {
    case TYPEID_INT8:
	*((PSFInt8 *)data) = *((int8_t *)buf+3);
	return 4;
    case TYPEID_INT32:
	*((PSFInt32 *)data) = GET_INT32(buf);
	return 4;
    case TYPEID_DOUBLE:
	GET_DOUBLE(*(PSFDouble *)data, buf);
	return 8;
    case TYPEID_COMPLEXDOUBLE:
	double re, im;
	GET_DOUBLE(re, buf); GET_DOUBLE(im, buf + 8); 
	*(PSFComplexDouble *)data = PSFComplexDouble(re, im);
	return 16;
    case TYPEID_STRUCT: 
	return ((Struct *)data)->deserialize(buf);
    default:
	throw UnknownType(datatypeid);    
    }
}

PSFScalar *DataTypeDef::new_scalar() const {
    switch(datatypeid) {
    case TYPEID_INT8:
	return new PSFInt8Scalar();
    case TYPEID_INT32:
	return new PSFInt32Scalar();
    case TYPEID_DOUBLE:
	return new PSFDoubleScalar();
    case TYPEID_COMPLEXDOUBLE:
	return new PSFComplexDoubleScalar();
    case TYPEID_STRUCT:
	return new StructScalar(Struct(structdef));
    default:
	throw UnknownType(datatypeid);    
    }
}

PSFVector *DataTypeDef::new_vector() const {
    switch(datatypeid) {
    case TYPEID_INT8:
	return new PSFInt8Vector();
    case TYPEID_INT32:
	return new PSFInt32Vector();
    case TYPEID_DOUBLE:
	return new PSFDoubleVector();
    case TYPEID_COMPLEXDOUBLE:
	return new PSFComplexDoubleVector();
    case TYPEID_STRUCT:
	return new StructVector(Struct(structdef));
    default:
	throw UnknownType(datatypeid);    
    }
}

//
// DataTypeRef
//

const DataTypeDef& DataTypeRef::get_def() const { 	
    return dynamic_cast<const DataTypeDef&>(psf->types->get_child(datatypeid)); 
}	

int DataTypeRef::deserialize(const char *buf) {	
    const char *startbuf = buf;

    buf += Chunk::deserialize(buf);
	
    id = GET_INT32(buf); buf+=4;
    buf += name.deserialize(buf);

    datatypeid = GET_INT32(buf); buf+=4;

    // Read optional properties
    while(true) {  	
	int chunktype = GET_INT32(buf);

	if(Property::ischunk(chunktype)) {
	    Property prop;
	    buf += prop.deserialize(buf);
	    properties.push_back(prop);
	} else
	    break;
    }

    return buf - startbuf;
}

void * DataTypeRef::new_dataobject() const {
    const DataTypeDef &def = dynamic_cast<const DataTypeDef&>(psf->types->get_child(datatypeid));
    return def.new_dataobject();
}

PSFVector * DataTypeRef::new_vector() const {
    const DataTypeDef &def = dynamic_cast<const DataTypeDef&>(psf->types->get_child(datatypeid));
    return def.new_vector();
}

const DataTypeDef& DataTypeRef::get_datatype() const { 
    return dynamic_cast<const DataTypeDef&>(psf->types->get_child(datatypeid)); 
}

int DataTypeRef::datasize() const {
    return get_datatype().datasize();
}
