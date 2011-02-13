#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

Chunk *HeaderSection::child_factory(int chunktype) const {
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

    for(Container::const_iterator ichild=begin(); ichild !=end(); ichild++) {
	Property *prop = (Property *)*ichild;
	properties[prop->get_name()] = prop->get_value();
    }

    return n;
}

const PSFScalar *HeaderSection::get_property(std::string key) const {
    if(properties.find(key) == properties.end())
	return NULL;
    else
	return properties.find(key)->second;
}

Chunk *TypeSection::child_factory(int chunktype) const {
    if(DataTypeDef::ischunk(chunktype))
	return new DataTypeDef();
    else {
	std::cerr << "Unexpected chunktype: " << chunktype << std::endl;
	throw IncorrectChunk(chunktype);
    }
}

Chunk * SweepSection::child_factory(int chunktype) const {
    if(DataTypeRef::ischunk(chunktype))
	return new DataTypeRef(psf);
    else if(chunktype == 3)
	return NULL;
    else {
	std::cerr << "Unexpected chunktype: " << chunktype << std::endl;
	throw IncorrectChunk(chunktype);
    }
}

