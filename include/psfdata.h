#ifndef _PSF_DATA
#define _PSF_DATA

#include <stdint.h>
#include <iostream>
#include <map>
#include <tr1/unordered_map>
#include <vector>

//
// Prototypes
//
class StructDef;
class GroupDef;
class PSFDataVector;

class PSFData {
public:	
    virtual int deserialize(const char *buf) { return 0; };
    virtual void print(std::ostream &stream)=0;
    
    virtual operator int() const { return -1; }
    
    virtual int datasize() const { throw NotImplemented(); }

    friend std::ostream &operator<<(std::ostream &stream, PSFData &o);
};

class String: public PSFData {
public:
    static const int type_id = 2;

    std::string value;
    String() {};

    void print(std::ostream &stream) {
	stream << value;
    };

    operator int() const { return atoi(value.c_str()); }

    int deserialize(const char *buf);
};


class Int8: public PSFData {
 private:
     int8_t value;
public:
    static const int type_id = 1;

    void print(std::ostream &stream) {
 	stream << value;
    };

     int datasize() const { return 4; }

     operator int() const { return value; }

     int deserialize(const char *buf);
};

class Int32: public PSFData {
 private:
     int32_t value;
public:
    static const int type_id = 5;

    void print(std::ostream &stream) {
 	stream << value;
    };

     int datasize() const { return 4; }

     operator int() const { return value; }

     int deserialize(const char *buf);
};

class UInt32: public PSFData {
private:
public:
    uint32_t value;
    UInt32() {};

    void print(std::ostream &stream) {
	stream << value;
    };

    int datasize() const { return 4; }

    operator int() const { return value; }

    int deserialize(const char *buf);
};

class Float64: public PSFData {
private:
public:
    double value;
    static const int type_id = 11;

    Float64() : value(0) {};
    Float64(double _value) : value(_value) {};

    void print(std::ostream &stream) {
	stream << value;
    };

    int datasize() const { return 8; }

    operator int() const { return int(value); }

    int deserialize(const char *buf);
};

class Struct: public PSFData, public std::tr1::unordered_map<std::string, PSFData *> {
 private:
    StructDef *structdef;
 public:
    static const int type_id = 16;

    Struct() : structdef(NULL) {}
    Struct(StructDef *_structdef) { structdef = _structdef; }

    int datasize() const;

    int deserialize(const char *buf);

    void print(std::ostream &stream);
};

class PSFDataVector {
 public:
    int type_id;
    
    virtual void append_value(PSFData *) {};
    
    virtual void extend(const PSFDataVector *) {};

    static PSFDataVector *create(int type_id);
};

class Group: public PSFData, public std::vector<PSFDataVector *> {
 private:
    GroupDef *groupdef;
    std::vector<int> *filter;
 public:
    void print(std::ostream &stream) {
	stream << groupdef;
    };
    Group(GroupDef *groupdef, std::vector<int> *filter=NULL);
    int datasize() const { return 0; }
    int deserialize(const char *buf, int n, int windowsize);
};

template<class T>
class PSFDataVectorT : public PSFDataVector, public std::vector<T> {
 public:
    void append_value(PSFData *x) { 	
	T &v = dynamic_cast<T &>(*x);
	push_back(v); 	
    }
    
    void extend(const PSFDataVector *vec) {
	const PSFDataVectorT& tvec = dynamic_cast<const PSFDataVectorT &>(*vec);
	reserve(std::vector<T>::size() + distance(tvec.begin(), tvec.end()));
	insert(std::vector<T>::end(), tvec.begin(), tvec.end());
    }
};

typedef PSFDataVectorT<Float64> Float64Vector;
typedef PSFDataVectorT<Struct> StructVector;

PSFData *psfdata_from_typeid(int type_id);
int psfdata_size(int type_id);

#endif
