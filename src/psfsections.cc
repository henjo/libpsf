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
      const Property *prop = dynamic_cast<const Property *>(*ichild);
      m_properties.append_prop(*prop);
    }

    return n;
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

