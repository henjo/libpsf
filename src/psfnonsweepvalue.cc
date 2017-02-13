#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

#include <assert.h>


Chunk * ValueSectionNonSweep::child_factory(int chunktype) const {
    if(NonSweepValue::ischunk(chunktype))
	return new NonSweepValue(psf);
    else {
	throw IncorrectChunk(chunktype);
    }
}

const PSFScalar& ValueSectionNonSweep::get_value(std::string name) const {
    return dynamic_cast<const NonSweepValue &>(get_child(name)).get_value();
}

const PropertyBlock & ValueSectionNonSweep::get_value_properties(const std::string name) const {
  const NonSweepValue &child = dynamic_cast<const NonSweepValue &>(get_child(name));
  return child.get_properties();
}

NonSweepValue::~NonSweepValue() {
    if(m_value)
	delete(m_value);
}

int NonSweepValue::deserialize(const char *buf) {
    const char *startbuf = buf;

    buf += Chunk::deserialize(buf);
   
    m_id = GET_INT32(buf); buf+=4;
    buf += m_name.deserialize(buf);	
    m_valuetypeid = GET_INT32(buf); buf+=4;
    
    const DataTypeDef& def = m_psf->get_type_section().get_typedef(m_valuetypeid);
    m_value = def.new_scalar();
    
    buf += def.deserialize_data(m_value->ptr(), buf);

    buf += m_propblock.deserialize(buf);

    return buf - startbuf;
}

