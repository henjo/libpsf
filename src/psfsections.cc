#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

Chunk *HeaderSection::child_factory(int chunktype) {
    if(Property::ischunk(chunktype))
	return new Property();
    else if(chunktype == 1)
	return NULL;
    else {
	std::cerr << "Unexpected chunktype: " << chunktype << std::endl;
	throw IncorrectChunk(chunktype);
    }
}

Chunk *TypeSection::child_factory(int chunktype) {
    if(DataTypeDef::ischunk(chunktype))
	return new DataTypeDef();
    else {
	std::cerr << "Unexpected chunktype: " << chunktype << std::endl;
	throw IncorrectChunk(chunktype);
    }
}

Chunk * SweepSection::child_factory(int chunktype) {
    if(DataTypeRef::ischunk(chunktype))
	return new DataTypeRef(psf);
    else if(chunktype == 3)
	return NULL;
    else {
	std::cerr << "Unexpected chunktype: " << chunktype << std::endl;
	throw IncorrectChunk(chunktype);
    }
}

