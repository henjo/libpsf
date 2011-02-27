#ifndef _PSF_DATA
#define _PSF_DATA

#include <stdint.h>
#include <complex>
#include <iostream>
#include <map>
#include <tr1/unordered_map>
#include <vector>
#include <sstream>

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

// 
// Composite data types
//
class Struct: public std::tr1::unordered_map<std::string, PSFScalar *> {
 private:
    const StructDef *structdef;
 public:
    Struct() : structdef(NULL) {}
    Struct(const StructDef *_structdef) { structdef = _structdef; }
    ~Struct();

    int datasize() const;

    int deserialize(const char *buf);

    friend std::ostream &operator<<(std::ostream &stream, const Struct &o);
};

/* class Group: public std::vector<PSFVector *> { */
/*  private: */
/*     const GroupDef *groupdef; */
/*     std::vector<int> *filter; */
/*  public: */
/*     Group(const GroupDef *groupdef, std::vector<int> *filter=NULL); */
/*     ~Group(); */
    
/*     void print(std::ostream &stream) const { */
/* 	stream << groupdef; */
/*     }; */
/*     int datasize() const { return 0; } */
/*     int deserialize(const char *buf, int n, int windowsize); */
/* }; */

//
// Scalar data
//
class PSFScalar {
 private:
    virtual void print(std::ostream &stream) const = 0;
 public:
    virtual ~PSFScalar() {};
    virtual int deserialize(const char *buf) { return 0; };

    virtual operator int() const { return -1; }
    virtual operator double() const { return 0.0; }
    virtual std::string tostring() const = 0;
    friend std::ostream &operator<<(std::ostream &stream, const PSFScalar &o);
    virtual void *ptr() = 0;
    virtual PSFScalar *clone() const = 0;
};

template<class T> 
class PSFScalarT : public PSFScalar {
 private:
    virtual void print(std::ostream &stream) const { stream << value; }
 public:
    T value;

    PSFScalarT() {};
    PSFScalarT(const T& init) : value(init) {};

    operator double() const;
    operator int() const;

    std::string tostring() const {
	std::stringstream out;
	out << value;
	return out.str();
    }

    int deserialize(const char *buf);
    void *ptr() { return &value; }
    PSFScalar* clone() const { return new PSFScalarT(*this); }
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
    virtual ~PSFVector() {};
    int type_id;

    virtual std::size_t size() const = 0;
    
    virtual void resize(std::size_t n)=0;
    
    virtual void append_value(void *) {};
    
    virtual void extend(const PSFVector *) {};

    virtual void *ptr_at(int i) = 0;
};

template<class T>
class PSFVectorT : public PSFVector, public std::vector<T> {
 private:
    T init;
 public:
    PSFVectorT();
    PSFVectorT(const T& _init) : init(_init) {};
    void extend(const PSFVector *vec) {
	const PSFVectorT& tvec = dynamic_cast<const PSFVectorT &>(*vec);
	reserve(std::vector<T>::size() + distance(tvec.begin(), tvec.end()));
	insert(std::vector<T>::end(), tvec.begin(), tvec.end());
    }
    
    void *ptr_at(int i) { return &std::vector<T>::at(i); }

    std::size_t size() const { return std::vector<T>::size(); };
    void resize(std::size_t n) { std::vector<T>::resize(n, init); };
};

typedef PSFVectorT<PSFDouble> PSFDoubleVector;
typedef PSFVectorT<PSFComplexDouble> PSFComplexDoubleVector;
typedef PSFVectorT<PSFInt8> PSFInt8Vector;
typedef PSFVectorT<PSFInt32> PSFInt32Vector;
typedef PSFVectorT<PSFString> PSFStringVector;
typedef PSFVectorT<Struct> StructVector;

int psfdata_size(int type_id);

#endif
