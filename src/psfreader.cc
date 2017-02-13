#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

#include <assert.h>


Chunk * ValueSectionNonSweep::child_factory(int chunktype) {
    if(NonSweepValue::ischunk(chunktype))
	return new NonSweepValue(psf);
    else {
	throw IncorrectChunk(chunktype);
    }
}

PSFData* ValueSectionNonSweep::get_value(std::string name) {
    return dynamic_cast<NonSweepValue &>(get_child(name)).get_value();
}

int NonSweepValue::deserialize(const char *buf) {
    const char *startbuf = buf;

    buf += Chunk::deserialize(buf);
    
    buf += id.deserialize(buf);
    buf += name.deserialize(buf);	
    buf += valuetypeid.deserialize(buf);
    
    value = psf->types->get_typedef(valuetypeid.value).get_data_object();

    buf += value->deserialize(buf);
    
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

