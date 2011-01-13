#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

int Chunk::deserialize(const char *buf) {
    uint32_t testtype = GET_INT32(buf);

    if( (chunktype != -1) && testtype != chunktype )
	throw IncorrectChunk(testtype);

    return 4;
}

std::ostream &operator<<(std::ostream &stream, Chunk &o)
{
    o.print(stream);
    return stream; // must return stream
}

