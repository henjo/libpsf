#ifndef _PSF
#define _PSF

#include <stdlib.h>

#include <exception>
#include <vector>
#include <string>

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

class PSFDataSet {
 private:
    PSFFile *psf;
    std::string filename;
 public:
    PSFDataSet(std::string filename);
    ~PSFDataSet();

    std::vector<std::string> get_signal_names();
    PSFDataVector *get_param_values();
    PSFDataVector *get_signal_values(std::string name);
    PSFData *get_signal_value(std::string name);
};

#endif
