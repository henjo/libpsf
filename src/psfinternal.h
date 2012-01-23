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

#ifdef HAVE_TR1_UNORDERED_MAP
#include <tr1/unordered_map>
#endif

#include <boost/iterator/iterator_facade.hpp>

#include "psfdata.h"
#include "psfendian.h"

//
// PSF constants
//
#define SECTION_HEADER 0
#define SECTION_TYPE 1
#define SECTION_SWEEP 2
#define SECTION_TRACE 3
#define SECTION_VALUE 4

#define CHUNKID_VALUESECTIONEND 15

#define GET_INT32(buf) ntohl(*((uint32_t *)(buf)))
#define GET_DOUBLE(dest, buf) *((uint64_t *)&(dest)) = be64toh(*((uint64_t *)(buf)))
#define READ_INT32(var, fstr) fstr.read((char *)&var, 4); var = ntohl(var)

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
class PSFScalar;
class SweepValue;
class SweepValueSimple;

//
// PSF data types
//
typedef std::vector<const Chunk *> ChildList;
typedef std::vector<Property> PropertyList;
typedef std::vector<int> OffsetList;
typedef std::vector<int> TraceIdx;
typedef std::vector<const Chunk *> Filter;
typedef std::vector<std::string> NameList;
typedef std::vector<SweepValue> SweepValueList;
typedef std::map<std::string, const PSFScalar *> PropertyMap;
#ifdef HAVE_TR1_UNORDERED_MAP
typedef std::tr1::unordered_map<int,int> TraceIDOffsetMap;
typedef std::tr1::unordered_map<std::string, int> NameIndexMap;
typedef std::tr1::unordered_map<std::string, int> NameIdMap;
typedef std::tr1::unordered_map<int, const Chunk *> IdMap;
#else
typedef std::map<int,int> TraceIDOffsetMap;
typedef std::map<std::string, int> NameIndexMap;
typedef std::map<std::string, int> NameIdMap;
typedef std::map<int, const Chunk *> IdMap;
#endif

class DataList : public std::vector<PSFScalar *> {
 public:
    ~DataList() { clear(); }
    void clear() {
	for(iterator i=begin(); i != end(); i++)
	    delete(*i);
	std::vector<PSFScalar *>::clear();
    }
};

//
// Data chunk classes
//

class Chunk {
public:	
    static const int type = -1;
    int chunktype;

    Chunk() { chunktype = -1; }
    virtual ~Chunk() {};

    virtual void print(std::ostream &stream) const {
	stream << "Chunk()";
    };

    virtual int deserialize(const char *buf);
    virtual int32_t get_id() const { return -1; };
    virtual const std::string& get_name() const { throw(NotImplemented()); }

    virtual void * new_dataobject() const { return NULL; };
    virtual PSFScalar* new_scalar() const { return NULL; };
    virtual const PSFVector* new_vector() const { return NULL; };
    virtual int datasize() const { return 0; };
};

std::ostream &operator<<(std::ostream &stream, const Chunk &o);

class Property: public Chunk {	
private:
    PSFStringScalar name;
    PSFScalar *value;
public:
    Property() : value(NULL) {}
    Property(Property const &);

    virtual ~Property();

    const std::string& get_name() const { return name.value; }

    const PSFScalar *get_value() const { return value; }
    
    static bool ischunk(int chunktype) {
	return (chunktype >= 33) && (chunktype <= 35);
    };

    virtual void print(std::ostream &stream) const {
	stream << "Property(" << name << "," << *value << ")";
    };
    
    virtual int deserialize(const char *buf);
};

class Container: public Chunk, public ChildList {
private:
public:
    virtual ~Container();

    virtual Chunk *child_factory(int chunktype) const {
	return NULL;
    }
    
    Chunk *deserialize_child(const char **buf);

    virtual NameList get_names() const;

    virtual void add_child(Chunk *child) { push_back(child); };
    virtual const Chunk & get_child(int id) const;
    virtual const Chunk & get_child(std::string name) const;

};

class SimpleContainer: public Container {
public:
    virtual int deserialize(const char *buf, int abspos);

    virtual void print(std::ostream &stream) const;
    
};

class SubContainer: public SimpleContainer {
private:
    Container *parent;
public:
    virtual Chunk *child_factory(int chunktype) const {
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
    IdMap idmap;
    NameIndexMap namemap;
 public:	
    virtual int deserialize(const char *buf, int abspos);

    virtual const Chunk & get_child(int id) const;
    virtual const Chunk & get_child(std::string name) const;
    virtual int get_child_index(std::string name) const;

    virtual void print(std::ostream &stream) const;
};

class DataTypeDef: public Container {
private:
 public:
    int id;
    PSFStringScalar name;
    int datatypeid;
    PropertyList properties;
    StructDef *structdef;
    int _datasize;
 public:
    static const int type = 16;

    DataTypeDef() : structdef(NULL) { chunktype = type; }
    ~DataTypeDef();

    void *new_dataobject() const;
    PSFScalar *new_scalar() const;
    PSFVector *new_vector() const;

    virtual const std::string& get_name() const { return name.value; }

    virtual Chunk *child_factory(int chunktype) const {
	if(Property::ischunk(chunktype))
	    return new Property();
	else
	    throw IncorrectChunk(chunktype);
    };	

    virtual int deserialize(const char *buf);
    
    int deserialize_data(void *data, const char *buf) const;

    int datasize() const { return _datasize; }

    virtual int32_t get_id() const { return id; };

    static bool ischunk(int chunktype) {
	return chunktype == DataTypeDef::type;
    };
    
};

class GroupDef: public Container {
private:
    int id;
    PSFStringScalar name;
    int nchildren;

    PSFFile *psf;

    virtual Chunk *child_factory(int chunktype) const;

    virtual int deserialize(const char *buf);

    TraceIDOffsetMap indexmap;
    NameIdMap namemap;
    
    void _create_valueindexmap();

public:
    static const int type = 17;

    GroupDef(PSFFile *_psf) : psf(_psf) { chunktype = type; }

    const std::string& get_name() const { return name.value; }

    virtual const Chunk & get_child(int id) const { return Container::get_child(id); }
    virtual const Chunk & get_child(std::string name) const;
    virtual int get_child_index(std::string name) const;

    //    void *new_dataobject() const;
    //    Group *new_group(std::vector<int> *filter) const;

    int fill_offsetmap(TraceIDOffsetMap& map, int windowsize=0, int startoffset=0) const;
    
    static bool ischunk(int chunktype) {
	return chunktype == GroupDef::type;
    };
    
    //    friend class Group;
};

class DataTypeRef: public Container {
private:
    int id;
    PSFStringScalar name;
    int datatypeid;
    PropertyList properties;
    StructDef *structdef;
    PSFFile *psf;
    
    virtual Chunk *child_factory(int chunktype) const {
	if(Property::ischunk(chunktype))
	    return new Property();
	else
	    throw IncorrectChunk(chunktype);
    };	
    
public:
    static const int type = 16;

    DataTypeRef(PSFFile *_psf) { chunktype = type; psf = _psf; }

    virtual int32_t get_id() const { return id; };

    virtual const std::string& get_name() const { return name.value; }

    const DataTypeDef& get_def() const;

    virtual int deserialize(const char *buf);

    const DataTypeDef& get_datatype() const;

    void *new_dataobject() const;
    PSFVector *new_vector() const;

    int deserialize_data(void *data, const char *buf) const { 
	return get_def().deserialize_data(data, buf); 
    }

    int datasize() const;

    static bool ischunk(int chunktype) {
	return chunktype == DataTypeDef::type;
    };
    
};

class StructDef: public Container {
 private:
    int _datasize;
public:
    virtual Chunk *child_factory(int chunktype) const;

    void* new_dataobject() const;
    
    int datasize() const { return _datasize; };

    virtual int deserialize(const char *buf);
};

//
// PSF file section classes
// 
class HeaderSection: public SimpleContainer {	
private:
    PropertyMap properties;
public:
    static const int type = 21;
    HeaderSection() {
	chunktype = HeaderSection::type;
    }

    virtual Chunk *child_factory(int chunktype) const;
    
    virtual int deserialize(const char *buf, int abspos);

    const PropertyMap& get_header_properties() const { return properties; }

    const PSFScalar *get_property(std::string key) const;
};

class TypeSection: public IndexedContainer {
public:	
    static const int type = 21;

    virtual Chunk *child_factory(int chunktype) const;

    const DataTypeDef& get_typedef(int id) const { 
	return dynamic_cast<const DataTypeDef &>(get_child(id));
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

    TraceSection(PSFFile *_psf) {
	chunktype = TypeSection::type;
	psf = _psf;
    }

    virtual Chunk *child_factory(int chunktype) const;

    NameList get_names() const;

    const DataTypeRef& get_trace_by_index(const TraceIdx &) const;
    const DataTypeRef& get_trace_by_name(std::string name) const;
    const TraceIdx get_traceindex_by_name(std::string name) const;
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

    SweepSection(PSFFile *_psf) {
	chunktype = SweepSection::type;
	psf = _psf;
    }

    virtual Chunk *child_factory(int chunktype) const;

    const DataTypeRef & get_sweep(int id) const {
	return dynamic_cast<const DataTypeRef &>(get_child(id));
    }

};

class NonSweepValue : public Chunk {
 private:
    int id;
    PSFStringScalar name;
    int valuetypeid;
    PSFScalar *value;
    PropertyList properties;
    PSFFile *psf;
 public:
    NonSweepValue(PSFFile *_psf) : psf(_psf), value(NULL) {};
    virtual ~NonSweepValue();

    static const int type = 16;

    const std::string& get_name() const { return name.value; }

    const PSFScalar& get_value() const { return *value; } 

    const PropertyList& get_properties() const { return properties; }

    virtual int deserialize(const char *buf);

    static bool ischunk(int chunktype) {
	return chunktype == type;
    }
};

class SweepValue: public Chunk, public std::vector<PSFVector *> {
 protected:
    int id;
    PSFStringScalar name;
    int linktypeid;
    PSFVector *paramvalues;

public:
    SweepValue() : paramvalues(NULL) { chunktype = type; }
    virtual ~SweepValue();

    static const int type = 16;

    const std::string& get_name() const { return name.value; }

    PSFVector *get_param_values(bool release=false);

    virtual int deserialize(const char *buf, int *n, int windowoffset, PSFFile *psf, Filter &filter) {};
};

class SweepValueSimple: public SweepValue {
 public:
    int deserialize(const char *buf, int *n, int windowoffset, PSFFile *psf, Filter &filter);
};

class SweepValueWindowed: public SweepValue {
 public:
    int deserialize(const char *buf, int *n, int windowoffset, PSFFile *psf, Filter &filter);
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

    const PSFScalar& get_value(std::string name) const;
    PropertyMap get_value_properties(const std::string name) const;
    
    virtual Chunk *child_factory(int chunktype) const;

};

class ValueSectionSweep: public Chunk {	
private:
    PSFFile *psf;

    int _valuesize;
    TraceIDOffsetMap offsetmap;
    void _create_valueoffsetmap(bool windowedsweep);
    
    const char *valuebuf, *endbuf;
    
    bool windowedsweep;

public:
    static const int type = 21;
    
    ValueSectionSweep(PSFFile *psf);

    virtual int deserialize(const char *buf, int abspos);
    
    // Allocate a value of correct class
    SweepValue *new_value() const;

    SweepValue *get_values(Filter &filter) const;
    PSFVector* get_values(std::string name) const;
    PSFVector* get_param_values() const;

    int valueoffset(int id) const;
    int valuesize() const { return _valuesize; };

    typedef SweepValueIterator<SweepValue> iterator;
    const iterator begin(SweepValue *, ChildList &filter) const ;
    const iterator end() const;
};


class PSFFile {	
 private:
    int fd;
    void deserialize(const char *buf, int size);
    char *buffer;
    int size;

public:	
    PSFFile(std::string _filename);
    ~PSFFile();
    
    std::string filename;
    HeaderSection *header;
    TypeSection *types;
    SweepSection *sweeps;
    TraceSection *traces;
    ValueSectionSweep *sweepvalues;
    ValueSectionNonSweep *nonsweepvalues;

    NameList get_param_names() const;
    PSFVector *get_param_values() const;
    PropertyMap get_value_properties(std::string name) const;
    PSFVector *get_values(std::string name) const;
    const PSFScalar& get_value(std::string name) const;
    
    NameList get_names() const;
    
    void open();
    void close();
    
    bool validate() const;
};

#endif
