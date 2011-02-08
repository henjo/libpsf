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

int HeaderSection::deserialize(const char *buf, int abspos) {
    int n = SimpleContainer::deserialize(buf, abspos);

    for(Container::const_iterator child=begin(); child !=end(); child++) {
	Property *prop = (Property *)*child;
	properties[prop->get_name()] = prop->get_value();
    }

    return n;
}

PSFScalar *HeaderSection::get_property(std::string key) {
    if(properties.find(key) == properties.end())
	return NULL;
    else
	return properties.find(key)->second;
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

