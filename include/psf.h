#ifndef _PSF
#define _PSF

#include <stdlib.h>

#include <exception>
#include <vector>
#include <string>
#include <map>

// Prototypes
class PSFFile;

//
// Exceptions 
// 
class IncorrectChunk: public std::exception {
public:		
    int chunktype;
    IncorrectChunk(int _chunktype) {
	chunktype = _chunktype;
    }
};

class UnknownType: public std::exception {
public:		
    int type_id;
    UnknownType(int _type_id) {
	type_id = _type_id;
    }
};

class NotImplemented: public std::exception {};
class FileOpenError: public std::exception {};

#include "psfdata.h"

typedef std::map<std::string, PSFScalar *> PropertyMap;

class PSFDataSet {
 private:
    PSFFile *psf;
    std::string filename;
 public:
    PSFDataSet(std::string filename);
    ~PSFDataSet();

    const PropertyMap get_header_properties();

    std::vector<std::string> get_signal_names();
    PSFVector *get_param_values();
    PSFVector *get_signal_values(std::string name);
    PSFScalar *get_signal_value(std::string name);
};

#endif
