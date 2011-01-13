#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

Chunk * StructDef::child_factory(int chunktype) {
    if(DataTypeDef::ischunk(chunktype))
	return new DataTypeDef();
    else if(chunktype == 18)
	return NULL;
    else
	throw IncorrectChunk(chunktype);
};	

int StructDef::deserialize(const char *buf) {
    const char *startbuf = buf;

    Chunk *child = NULL;
    _datasize = 0;
    do {
	child = deserialize_child(&buf);  
	if(child) {
	    add_child(child);
	    _datasize += ((DataTypeDef *)child)->datasize();
	}
    } while (child);

    return buf - startbuf;
}

Struct * StructDef::get_data_object() {
    return new Struct(this);
}

