#ifndef _PSF_DATA
#define _PSF_DATA

#include <stdint.h>
#include <complex>
#include <iostream>
#include <map>
#include <tr1/unordered_map>
#include <vector>

//
// Prototypes
//
class StructDef;
class GroupDef;
class PSFScalar;
class PSFVector;

//
// Type identifiers
//
const int TYPEID_INT8 = 1;
const int TYPEID_STRING = 2;
const int TYPEID_ARRAY = 3;
const int TYPEID_INT32 = 5;
const int TYPEID_DOUBLE = 11;
const int TYPEID_COMPLEXDOUBLE = 12;
const int TYPEID_STRUCT = 16;

// Data types
typedef int8_t PSFInt8;
typedef int32_t PSFInt32;
typedef double PSFDouble;
typedef std::complex<double> PSFComplexDouble;
typedef std::string PSFString;

class Struct: public std::tr1::unordered_map<std::string, PSFScalar *> {
 private:
    StructDef *structdef;
 public:
    Struct() : structdef(NULL) {}
    Struct(StructDef *_structdef) { structdef = _structdef; }

    int datasize() const;

    int deserialize(const char *buf);

    friend std::ostream &operator<<(std::ostream &stream, Struct &o);
};

class Group: public std::vector<PSFVector *> {
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

//
// Scalar data
//
class PSFScalar {
 private:
    virtual void print(std::ostream &stream)=0;
 public:
    virtual int deserialize(const char *buf) { return 0; };

    virtual operator int() const { return -1; }
    virtual operator double() const { return 0.0; }
    friend std::ostream &operator<<(std::ostream &stream, PSFScalar &o);
    static PSFScalar *create(int type_id);
};

template<class T> 
class PSFScalarT : public PSFScalar {
 private:
    virtual void print(std::ostream &stream) { stream << value; }
 public:
    T value;

    operator double() const;
    operator int() const;
    int deserialize(const char *buf);
};

typedef PSFScalarT<PSFDouble> PSFDoubleScalar;
typedef PSFScalarT<PSFComplexDouble> PSFComplexDoubleScalar;
typedef PSFScalarT<PSFInt8> PSFInt8Scalar;
typedef PSFScalarT<PSFInt32> PSFInt32Scalar;
typedef PSFScalarT<PSFString> PSFStringScalar;
typedef PSFScalarT<Struct> StructScalar;

//
// Vector data
//
class PSFVector {
 public:
    int type_id;

    virtual std::size_t size()=0;
    
    virtual void resize(std::size_t n)=0;
    
    virtual void append_value(void *) {};
    
    virtual void extend(const PSFVector *) {};

    static PSFVector *create(int type_id);

    virtual void *ptr_at(int i)=0;
};

template<class T>
class PSFVectorT : public PSFVector, public std::vector<T> {
 public:
    void extend(const PSFVector *vec) {
	const PSFVectorT& tvec = dynamic_cast<const PSFVectorT &>(*vec);
	reserve(std::vector<T>::size() + distance(tvec.begin(), tvec.end()));
	insert(std::vector<T>::end(), tvec.begin(), tvec.end());
    }
    
    void *ptr_at(int i) { return &std::vector<T>::at(i); }

    std::size_t size() { return std::vector<T>::size(); };
    void resize(std::size_t n) { std::vector<T>::resize(n); };
    
    
};

typedef PSFVectorT<PSFDouble> PSFDoubleVector;
typedef PSFVectorT<PSFComplexDouble> PSFComplexDoubleVector;
typedef PSFVectorT<PSFInt8> PSFInt8Vector;
typedef PSFVectorT<PSFInt32> PSFInt32Vector;
typedef PSFVectorT<PSFString> PSFStringVector;
typedef PSFVectorT<Struct> StructVector;

int psfdata_size(int type_id);

#endif
