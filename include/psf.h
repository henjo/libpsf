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
class NotFound: public std::exception {};

#include "psfdata.h"

typedef std::map<std::string, const PSFScalar *> PropertyMap;

class PSFDataSet {
 private:
    PSFFile *psf;
    std::string filename;
 public:
    PSFDataSet(std::string filename);
    ~PSFDataSet();

    const PropertyMap& get_header_properties() const;

    const std::vector<std::string> get_signal_names() const;
    int get_sweep_npoints() const;
    PSFVector *get_sweep_values() const;
    PropertyMap get_signal_properties(std::string name) const;
    PSFVector *get_signal_vector(std::string name) const ;
    const PSFScalar *get_signal_scalar(std::string name) const;
};

#endif
