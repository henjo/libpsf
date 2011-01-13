#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

//
// DataTypeDef
//


int DataTypeDef::deserialize(const char *buf) {
    const char *startbuf = buf;

    buf += Chunk::deserialize(buf);
	
    buf += id.deserialize(buf);
    buf += name.deserialize(buf);
	
    UInt32 arraytype;
    buf += arraytype.deserialize(buf);

    buf += datatypeid.deserialize(buf);

    if(datatypeid.value == 16) {
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


PSFData *DataTypeDef::get_data_object() {
    if(datatypeid == 16) 
	return structdef->get_data_object();
    else
	return psfdata_from_typeid(datatypeid);
}

PSFDataVector *DataTypeDef::get_data_vector() {
    return PSFDataVector::create(datatypeid);
}

//
// DataTypeRef
//


int DataTypeRef::deserialize(const char *buf) {	
    const char *startbuf = buf;

    buf += Chunk::deserialize(buf);
	
    buf += id.deserialize(buf);
    buf += name.deserialize(buf);

    buf += datatypeid.deserialize(buf);

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
 
PSFData * DataTypeRef::get_data_object() {
    DataTypeDef &def = dynamic_cast<DataTypeDef&>(psf->types->get_child(datatypeid));
    return def.get_data_object();
}

PSFDataVector * DataTypeRef::get_data_vector() {
    DataTypeDef &def = dynamic_cast<DataTypeDef&>(psf->types->get_child(datatypeid));
    return def.get_data_vector();
}

DataTypeDef& DataTypeRef::get_datatype() { 
    return dynamic_cast<DataTypeDef&>(psf->types->get_child(datatypeid)); 
}

int DataTypeRef::datasize() {
    return get_datatype().datasize();
}
