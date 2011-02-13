#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

#include <assert.h>


Chunk * ValueSectionNonSweep::child_factory(int chunktype) const {
    if(NonSweepValue::ischunk(chunktype))
	return new NonSweepValue(psf);
    else {
	std::cerr << "Unexpected chunktype: " << chunktype << std::endl;
	throw IncorrectChunk(chunktype);
    }
}

const PSFScalar* ValueSectionNonSweep::get_value(std::string name) const {
    return dynamic_cast<const NonSweepValue &>(get_child(name)).get_value();
}

PropertyMap ValueSectionNonSweep::get_value_properties(const std::string name) const {
    const PropertyList& plist = dynamic_cast<const NonSweepValue &>(get_child(name)).get_properties();
    PropertyMap pmap;

    for(PropertyList::const_iterator i = plist.begin(); i != plist.end(); i++)
	pmap[i->get_name()] = i->get_value();

    return pmap;
}

NonSweepValue::~NonSweepValue() {
    if(value)
	delete(value);
}

int NonSweepValue::deserialize(const char *buf) {
    const char *startbuf = buf;

    buf += Chunk::deserialize(buf);
   
    id = GET_INT32(buf); buf+=4;
    buf += name.deserialize(buf);	
    valuetypeid = GET_INT32(buf); buf+=4;
    
    const DataTypeDef& def = psf->types->get_typedef(valuetypeid);
    value = def.new_scalar();
    
    buf += def.deserialize_data(value->ptr(), buf);
    
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

