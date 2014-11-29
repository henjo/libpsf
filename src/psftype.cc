#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

//
// DataTypeDef
//

DataTypeDef::~DataTypeDef() {
    if(m_structdef)
	delete(m_structdef);
}

int DataTypeDef::deserialize(const char *buf) {
    const char *startbuf = buf;

    buf += Chunk::deserialize(buf);

    m_id = GET_INT32(buf); buf+=4;

    buf += m_name.deserialize(buf);
	
    int arraytype = GET_INT32(buf); buf+=4;
    
    m_datatypeid = GET_INT32(buf);
    buf += 4;

    if(m_datatypeid == 16) {
	m_structdef = new StructDef();
	buf += m_structdef->deserialize(buf);
	_datasize = m_structdef->datasize();
    } else
	_datasize = psfdata_size(m_datatypeid);

    buf += m_properties.deserialize(buf);

    return buf - startbuf;
};

void * DataTypeDef::new_dataobject() const {
    switch(m_datatypeid) {
    case TYPEID_INT8:
	return new PSFInt8;
    case TYPEID_INT32:
	return new PSFInt32;
    case TYPEID_DOUBLE:
	return new PSFDouble;
    case TYPEID_STRUCT:
	return m_structdef->new_dataobject();
    default:
	throw UnknownType(m_datatypeid);    
    }
}

int DataTypeDef::deserialize_data(void *data, const char *buf) const {
    switch(m_datatypeid) {
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
	throw UnknownType(m_datatypeid);    
    }
}

PSFScalar *DataTypeDef::new_scalar() const {
    switch(m_datatypeid) {
    case TYPEID_INT8:
	return new PSFInt8Scalar();
    case TYPEID_INT32:
	return new PSFInt32Scalar();
    case TYPEID_DOUBLE:
	return new PSFDoubleScalar();
    case TYPEID_COMPLEXDOUBLE:
	return new PSFComplexDoubleScalar();
    case TYPEID_STRUCT:
	return new StructScalar(Struct(m_structdef));
    default:
      throw UnknownType(m_datatypeid);    
    }
}

PSFVector *DataTypeDef::new_vector() const {
    switch(m_datatypeid) {
    case TYPEID_INT8:
	return new PSFInt8Vector();
    case TYPEID_INT32:
	return new PSFInt32Vector();
    case TYPEID_DOUBLE:
	return new PSFDoubleVector();
    case TYPEID_COMPLEXDOUBLE:
	return new PSFComplexDoubleVector();
    case TYPEID_STRUCT:
	return new StructVector(Struct(m_structdef));
    default:
	throw UnknownType(m_datatypeid);    
    }
}

//
// DataTypeRef
//

const DataTypeDef& DataTypeRef::get_def() const { 	
    return dynamic_cast<const DataTypeDef&>(m_psf->get_type_section().get_child(m_datatypeid)); 
}	

int DataTypeRef::deserialize(const char *buf) {	
    const char *startbuf = buf;

    buf += Chunk::deserialize(buf);
	
    m_id = GET_INT32(buf); buf+=4;
    buf += m_name.deserialize(buf);

    m_datatypeid = GET_INT32(buf); buf+=4;

    buf += m_properties.deserialize(buf);

    return buf - startbuf;
}

void * DataTypeRef::new_dataobject() const {
    const DataTypeDef &def = dynamic_cast<const DataTypeDef&>(m_psf->get_type_section().get_child(m_datatypeid));
    return def.new_dataobject();
}

PSFVector * DataTypeRef::new_vector() const {
    const DataTypeDef &def = dynamic_cast<const DataTypeDef&>(m_psf->get_type_section().get_child(m_datatypeid));
    return def.new_vector();
}

const DataTypeDef& DataTypeRef::get_datatype() const { 
    return dynamic_cast<const DataTypeDef&>(m_psf->get_type_section().get_child(m_datatypeid)); 
}

int DataTypeRef::datasize() const {
    return get_datatype().datasize();
}
