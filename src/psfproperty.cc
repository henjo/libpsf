#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

Property::~Property() {
  if(m_value)
    delete(m_value);
}

Property::Property(Property const &x) {
  if(x.m_value) {
    m_name  = x.m_name;
    m_value = x.m_value->clone();
  } else
    m_value = NULL;
}

int Property::deserialize(const char *buf) {
    const char *startbuf = buf;

    m_chunktype = GET_INT32(buf);
    buf += 4;

    if(!ischunk(m_chunktype))
	throw IncorrectChunk(m_chunktype);

    buf += m_name.deserialize(buf);

    switch(m_chunktype) {
    case 33:
	m_value = new PSFStringScalar;
	buf += m_value->deserialize(buf);
	break;
    case 34:
	m_value = new PSFInt32Scalar();
	buf += m_value->deserialize(buf);
	break;
    case 35:
	m_value = new PSFDoubleScalar();
	buf += m_value->deserialize(buf);
	break;
    }
    return buf - startbuf;
}

