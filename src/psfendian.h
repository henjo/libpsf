
// Define be64toh if not already defined
// This is needed for older glibc versions
#ifndef be64toh
inline long be64toh(long val) {
    unsigned char* c = (unsigned char*)&val;
    return long( (long((c[0] & 255)) << 56) + 
		 (long(c[1] & 255) << 48) +
		 (long(c[2] & 255) << 40) + 
		 (long(c[3] & 255) << 32) +
		 (long(c[4] & 255) << 24) +
		 (long(c[5] & 255) << 16) +
		 (long(c[6] & 255) <<  8) +
		 (long(c[7] & 255)      ) );
}
#endif
