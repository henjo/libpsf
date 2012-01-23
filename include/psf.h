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
class InvalidFileError: public std::exception {};
class FileCloseError: public std::exception {};
class NotFound: public std::exception {};

#include "psfdata.h"

typedef std::map<std::string, const PSFScalar *> PropertyMap;

class PSFDataSet {
 private:
    PSFFile *psf;
    std::string filename;
    bool invertstruct;
    bool _open;
 public:
    PSFDataSet(std::string filename);
    ~PSFDataSet();
    void close() ;
    void open() ;

    const PropertyMap& get_header_properties();

    const std::vector<std::string> get_signal_names();
    bool is_swept();

    int get_nsweeps();
    const std::vector<std::string> get_sweep_param_names();
    int get_sweep_npoints();
    PSFVector *get_sweep_values();

    PropertyMap get_signal_properties(std::string name);
    PSFBase *get_signal(std::string name);
    PSFVector *get_signal_vector(std::string name) ;
    const PSFScalar& get_signal_scalar(std::string name);

    void set_invertstruct(bool value) { invertstruct = value; }
    bool get_invertstruct() { return invertstruct; }
};

#endif
