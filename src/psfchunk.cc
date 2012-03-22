#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

int Chunk::deserialize(const char *buf) {
    uint32_t testtype = GET_INT32(buf);

    if( (m_chunktype != -1) && testtype != m_chunktype )
	throw IncorrectChunk(testtype);

    return 4;
}

std::ostream &operator<<(std::ostream &stream, const Chunk &o)
{
    o.print(stream);
    return stream; // must return stream
}

