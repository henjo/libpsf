#ifndef _PSF_INTERNAL
#define _PSF_INTERNAL

#include <stdint.h>
#include <arpa/inet.h>
#include <endian.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <tr1/unordered_map>

#include <boost/iterator/iterator_facade.hpp>

#include "psfdata.h"

//
// PSF constants
//
#define SECTION_HEADER 0
#define SECTION_TYPE 1
#define SECTION_SWEEP 2
#define SECTION_TRACE 3
#define SECTION_VALUE 4

#define CHUNKID_VALUESECTIONEND 15

#define GET_INT32(buf) ntohl(*((uint32_t *)(buf)));
#define READ_INT32(var, fstr) fstr.read((char *)&var, 4); var = ntohl(var);

typedef struct {
    uint32_t n;
    uint32_t offset;
    int size;
} Section;

//
// Prototypes
//
class Chunk;
class Property;
class PSFFile;
class StructDef;
class DataTypeRef;
class PSFData;
class SweepValue;
class SweepValueSimple;

//
// PSF data types
//
typedef std::vector<Chunk *> ChildList;
typedef std::vector<Property> PropertyList;
typedef std::tr1::unordered_map<int,int> TraceIDOffsetMap;
typedef std::vector<int> TraceIDList;
typedef std::vector<std::string> NameList;
typedef std::vector<SweepValue> SweepValueList;


class DataList : public std::vector<PSFData *> {
 public:
    ~DataList() { clear(); }
    void clear() {
	for(iterator i=begin(); i != end(); i++)
	    delete(*i);
	std::vector<PSFData *>::clear();
    }
};

//
// Data chunk classes
//

class Chunk {
public:	
    static const int type = -1;
    int chunktype;


    Chunk() {
	chunktype = -1;
    }

    virtual void print(std::ostream &stream) {
	stream << "Chunk()";
    };

    virtual int deserialize(const char *buf);
    virtual int32_t get_id() { return -1; };
    virtual std::string& get_name() { throw(NotImplemented()); }

    virtual PSFData* get_data_object() { return NULL; };
    virtual PSFDataVector* get_data_vector() { return NULL; };
    virtual int datasize() { return 0; };
};

std::ostream &operator<<(std::ostream &stream, Chunk &o);

class Property: public Chunk {	
private:
    String name;
    PSFData *value;
public:
    virtual std::string& get_name() { return name.value; }

    PSFData *get_value() { return value; }
    
    static bool ischunk(int chunktype) {
	return (chunktype >= 33) && (chunktype <= 35);
    };

    virtual void print(std::ostream &stream) {
	stream << "Property(" << name << "," << *value << ")";
    };
    
    virtual int deserialize(const char *buf);
};

class Container: public Chunk, public ChildList {
private:
public:
    virtual Chunk *child_factory(int chunktype) {
	return NULL;
    }
    
    Chunk *deserialize_child(const char **buf);

    virtual NameList get_names();

    virtual void add_child(Chunk *child) { push_back(child); };
    virtual Chunk & get_child(int id);
    virtual Chunk & get_child(std::string name);

    virtual ~Container() {
	//    	for(int i=0; i<size(); i++)
	//    	    delete (*this)[i];
    }

};

class SimpleContainer: public Container {
public:
    virtual int deserialize(const char *buf, int abspos);

    virtual void print(std::ostream &stream);
    
};

class SubContainer: public SimpleContainer {
private:
    Container *parent;
public:
    virtual Chunk *child_factory(int chunktype) {
	return parent->child_factory(chunktype);
    };

    SubContainer(Container *_parent) {
	parent = _parent;
    };
};

class Index: public Chunk {
 public:
    static const int type = 19;

    Index() { chunktype = Index::type; };

    virtual int deserialize(const char *buf);
};

class TraceIndex: public Chunk {
 public:
    static const int type = 19;

    TraceIndex() { chunktype = Index::type; };

    virtual int deserialize(const char *buf);
};

class IndexedContainer: public Container {
 private:
    std::tr1::unordered_map<int, Chunk *> idmap;
    std::tr1::unordered_map<std::string, Chunk *> namemap;
 public:	
    virtual int deserialize(const char *buf, int abspos);

    virtual Chunk & get_child(int id);
    virtual Chunk & get_child(std::string name);

    virtual void print(std::ostream &stream);
};

class DataTypeDef: public Container {
private:
 public:
    UInt32 id;
    String name;
    UInt32 datatypeid;
    PropertyList properties;
    StructDef *structdef;
    int _datasize;
 public:
    static const int type = 16;

    DataTypeDef() { chunktype = type; }

    PSFData* get_data_object();
    PSFDataVector *get_data_vector();

    virtual std::string& get_name() { return name.value; }

    virtual Chunk *child_factory(int chunktype) {
	if(Property::ischunk(chunktype))
	    return new Property();
	else
	    throw IncorrectChunk(chunktype);
    };	

    virtual int deserialize(const char *buf);
    
    int datasize() const { return _datasize; }

    virtual int32_t get_id() { return id; };

    static bool ischunk(int chunktype) {
	return chunktype == DataTypeDef::type;
    };
    
};

class GroupDef: public Container {
private:
    UInt32 id;
    String name;
    UInt32 nchildren;

    PSFFile *psf;

    virtual Chunk *child_factory(int chunktype);

    virtual int deserialize(const char *buf);

    TraceIDOffsetMap indexmap;
    void _create_valueoffsetmap();

public:
    static const int type = 17;

    GroupDef(PSFFile *_psf) : psf(_psf) { chunktype = type; }

    std::string& get_name() { return name.value; }

    PSFData *get_data_object();
    PSFData *get_data_object(std::vector<int> *filter);

    static bool ischunk(int chunktype) {
	return chunktype == GroupDef::type;
    };
    
    friend class Group;
};

class DataTypeRef: public Container {
private:
    UInt32 id;
    String name;
    UInt32 datatypeid;
    PropertyList properties;
    StructDef *structdef;
    PSFFile *psf;
    
    virtual Chunk *child_factory(int chunktype) {
	if(Property::ischunk(chunktype))
	    return new Property();
	else
	    throw IncorrectChunk(chunktype);
    };	
    
public:
    static const int type = 16;

    DataTypeRef(PSFFile *_psf) { chunktype = type; psf = _psf; }

    virtual int32_t get_id() { return id; };

    virtual std::string& get_name() { return name.value; }

    virtual int deserialize(const char *buf);

    DataTypeDef& get_datatype();

    PSFData *get_data_object();
    PSFDataVector *get_data_vector();

    int datasize();

    static bool ischunk(int chunktype) {
	return chunktype == DataTypeDef::type;
    };
    
};

class StructDef: public Container {
 private:
    int _datasize;
public:
    virtual Chunk *child_factory(int chunktype);

    Struct *get_data_object();
    
    int datasize() { return _datasize; };

    virtual int deserialize(const char *buf);
};

//
// PSF file section classes
// 

class HeaderSection: public SimpleContainer {	
private:
    std::map<std::string, PSFData *> properties;
public:
    static const int type = 21;
    HeaderSection() {
	chunktype = HeaderSection::type;
    }

    virtual Chunk *child_factory(int chunktype);
    
    virtual int deserialize(const char *buf, int abspos);

    PSFData *get_property(std::string key) {
	return properties.find(key)->second;
    }

};

class TypeSection: public IndexedContainer {
public:	
    static const int type = 21;

    virtual Chunk *child_factory(int chunktype);

    DataTypeDef& get_typedef(int id) { 
	return dynamic_cast<DataTypeDef &>(get_child(id));
    }

    TypeSection() {
	chunktype = TypeSection::type;
    }
};

class TraceSection: public IndexedContainer {
 private:
    PSFFile *psf;
    
public:	
    static const int type = 21;

    virtual Chunk *child_factory(int chunktype);

    NameList get_names();
    DataTypeRef& get_trace_by_name(std::string name);

    TraceSection(PSFFile *_psf) {
	chunktype = TypeSection::type;
	psf = _psf;
    }
};

class ZeroPad: public Chunk {
 public:
    static const int type = 20;

    ZeroPad() { chunktype = type; }

    int deserialize(const char *buf) {
	const char *startbuf = buf;

	buf += Chunk::deserialize(buf);
	
	uint32_t size = GET_INT32(buf);
	buf += sizeof(uint32_t) + size;
	
	return buf - startbuf;
    }
};	

class SweepSection: public SimpleContainer {	
 private:
    PSFFile *psf;
 public:
    static const int type = 21;
    virtual Chunk *child_factory(int chunktype);

    
    DataTypeRef & get_sweep(int id) {
	return dynamic_cast<DataTypeRef &>(get_child(id));
    }

    SweepSection(PSFFile *_psf) {
	chunktype = SweepSection::type;
	psf = _psf;
    }
};

class NonSweepValue : public Chunk {
 private:
    UInt32 id;
    String name;
    UInt32 valuetypeid;
    DataTypeDef *valuetype;	
    PSFData *value;
    PropertyList properties;
    PSFFile *psf;
 public:
    NonSweepValue(PSFFile *_psf) : psf(_psf) {};

    static const int type = 16;

    std::string& get_name() { return name.value; }

    PSFData* get_value() { return value; }

    virtual int deserialize(const char *buf);

    static bool ischunk(int chunktype) {
	return chunktype == type;
    }
};

class SweepValue: public Chunk, public std::vector<PSFDataVector *> {
 protected:
    UInt32 id;
    String name;
    UInt32 linktypeid;
    DataTypeRef paramtype;	
    PSFDataVector *paramvalues;

public:
    static const int type = 16;

    SweepValue() : paramvalues(NULL), paramtype(NULL) { chunktype = type; }

    std::string& get_name() { return name.value; }

    PSFDataVector *get_param_values() const { return paramvalues; }
    virtual int deserialize(const char *buf, int *n, int windowoffset, PSFFile *psf, ChildList &filter) {};
};

class SweepValueSimple: public SweepValue {
 public:
    int deserialize(const char *buf, int *n, int windowoffset, PSFFile *psf, ChildList &filter);
};

class SweepValueWindowed: public SweepValue {
 public:
    int deserialize(const char *buf, int *n, int windowoffset, PSFFile *psf, ChildList &filter);
};    

template <class T>
class SweepValueIterator : 	
public boost::iterator_facade<SweepValueIterator<T>, T, boost::forward_traversal_tag> {
 public:
 SweepValueIterator(T *_v, const char *valuebuf, PSFFile *_psf, ChildList *_filter) : 
    v(_v), psf(_psf), filter(_filter) {
	buf = valuebuf;
	nextbuf = buf + deserialize();
    }
    
 private:
    friend class boost::iterator_core_access;

    void increment() {
	buf = nextbuf;
	nextbuf = buf + deserialize();
    }

    T& dereference() const { return *v; }	

    bool equal(SweepValueIterator const & other) const { return this->buf == other.buf; }

    int deserialize();
    
    const char *buf, *nextbuf;
    T *v;
    ChildList *filter;
    PSFFile *psf;
};

class ValueSectionNonSweep: public IndexedContainer {
 public:
    static const int type = 21;
    PSFFile *psf;
    
    ValueSectionNonSweep(PSFFile *_psf) : psf(_psf) {};

    PSFData* get_value(std::string name);
   
    virtual Chunk *child_factory(int chunktype);

};

class ValueSectionSweep: public Chunk {	
private:
    PSFFile *psf;

    int _valuesize;
    TraceIDOffsetMap valueoffsetmap;
    void _create_valueoffsetmap(bool windowedsweep);
    
    const char *valuebuf, *endbuf;
    
    SweepValue *sweepvalue;

    bool windowedsweep;

public:
    static const int type = 21;
    
    ValueSectionSweep(PSFFile *psf);

    virtual int deserialize(const char *buf, int abspos);
    
    // Allocate a value of correct class
    SweepValue *new_value();

    SweepValue *get_values(ChildList &filter);
    PSFDataVector* get_values(std::string name);
    PSFDataVector* get_param_values();

    int valueoffset(int id);
    int valuesize() { return _valuesize; };

    typedef SweepValueIterator<SweepValue> iterator;
    const iterator begin(SweepValue *, ChildList &filter) const ;
    const iterator end() const;
};


class PSFFile {	
 private:
    int fd;
    void deserialize(const char *buf, int size);
  
public:	
    std::string filename;
    HeaderSection *header;
    TypeSection *types;
    SweepSection *sweeps;
    TraceSection *traces;
    ValueSectionSweep *sweepvalues;
    ValueSectionNonSweep *nonsweepvalues;

    PSFFile(std::string _filename);

    PSFDataVector *get_param_values();
    PSFDataVector *get_values(std::string name);
    PSFData* get_value(std::string name);
    
    NameList get_names();
    
    void open();
    void close();
    
    bool validate();
};

#endif
