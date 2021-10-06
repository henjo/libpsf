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
typedef std::map<int, Section> SectionMap;
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

    Chunk() { m_chunktype = -1; }
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

    int m_chunktype;
};

std::ostream &operator<<(std::ostream &stream, const Chunk &o);

class Property: public Chunk {	
public:
    Property() : m_value(NULL) {}
    Property(Property const &);

    virtual ~Property();

    const std::string& get_name() const { return m_name.value; }

    const PSFScalar *get_value() const { return m_value; }
    
    static bool ischunk(int chunktype) {
	return (chunktype >= 33) && (chunktype <= 35);
    };

    virtual void print(std::ostream &stream) const {
	stream << "Property(" << m_name << "," << *m_value << ")";
    };
    
    virtual int deserialize(const char *buf);

private:
    PSFStringScalar m_name;
    PSFScalar *m_value;
};

class PropertyBlock {
 public:
  PropertyBlock() {};

  const PSFScalar &find(const std::string key) const;
  bool hasprop(const std::string key) const;
  const Property &findprop(const std::string key) const;
  const PropertyMap &get_propmap() const { return m_propmap; } 
  void append_prop(const Property &prop);
  
  virtual int deserialize(const char *buf);

 private:
  PropertyList m_proplist; 
  PropertyMap m_propmap;
};

class Container: public Chunk, public ChildList {
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
public:
    virtual Chunk *child_factory(int chunktype) const {
	return parent->child_factory(chunktype);
    };

    SubContainer(Container *_parent) {
	parent = _parent;
    };

private:
    Container *parent;
};

class Index: public Chunk {
 public:
    static const int type = 19;

    Index() { m_chunktype = Index::type; };

    virtual int deserialize(const char *buf);
};

class TraceIndex: public Chunk {
 public:
    static const int type = 19;

    TraceIndex() { m_chunktype = Index::type; };

    virtual int deserialize(const char *buf);
};

class IndexedContainer: public Container {
 public:	
    virtual int deserialize(const char *buf, int abspos);

    virtual const Chunk & get_child(int id) const;
    virtual const Chunk & get_child(std::string name) const;
    virtual int get_child_index(std::string name) const;

    virtual void print(std::ostream &stream) const;

 private:
    IdMap idmap;
    NameIndexMap namemap;
};

class DataTypeDef: public Container {
 public:
    static const int type = 16;

    DataTypeDef() : m_structdef(NULL) { m_chunktype = type; }
    ~DataTypeDef();

    void *new_dataobject() const;
    PSFScalar *new_scalar() const;
    PSFVector *new_vector() const;

    virtual const std::string& get_name() const { return m_name.value; }

    const PropertyBlock& get_properties() const { return m_properties; }

    virtual Chunk *child_factory(int chunktype) const {
	if(Property::ischunk(chunktype))
	    return new Property();
	else
	    throw IncorrectChunk(chunktype);
    };	

    virtual int deserialize(const char *buf);
    
    int deserialize_data(void *data, const char *buf) const;

    int datasize() const { return _datasize; }

    virtual int32_t get_id() const { return m_id; };

    static bool ischunk(int chunktype) {
	return chunktype == DataTypeDef::type;
    };
    
private:
 public:
    int m_id;
    PSFStringScalar m_name;
    int m_datatypeid;
    PropertyBlock m_properties;
    StructDef *m_structdef;
    int _datasize;
};

class GroupDef: public Container {
public:
    static const int type = 17;

    GroupDef(PSFFile *psf) : m_psf(psf) { m_chunktype = type; }

    const std::string& get_name() const { return m_name.value; }

    virtual const Chunk & get_child(int id) const { return Container::get_child(id); }
    virtual const Chunk & get_child(std::string name) const;
    virtual int get_child_index(std::string name) const;

    int fill_offsetmap(TraceIDOffsetMap& map, int windowsize=0, int startoffset=0) const;
    
    static bool ischunk(int chunktype) {
	return GroupDef::type == chunktype;
    };
    
private:
    virtual Chunk *child_factory(int chunktype) const;

    virtual int deserialize(const char *buf);

    void _create_valueindexmap();

    int m_id;
    PSFStringScalar m_name;
    int m_nchildren;
    PSFFile *m_psf;
    TraceIDOffsetMap m_indexmap;
    NameIdMap m_namemap;
};

class DataTypeRef: public Container {
public:
    static const int type = 16;

    DataTypeRef(PSFFile *_psf) { m_chunktype = type; m_psf = _psf; }

    virtual int32_t get_id() const { return m_id; };

    virtual const std::string& get_name() const { return m_name.value; }

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
    
private:
    virtual Chunk *child_factory(int chunktype) const {
	if(Property::ischunk(chunktype))
	    return new Property();
	else
	    throw IncorrectChunk(chunktype);
    };	
    
    int m_id;
    PSFStringScalar m_name;
    int m_datatypeid;
    PropertyBlock m_properties;
    StructDef *m_structdef;
    PSFFile *m_psf;
};

class StructDef: public Container {
 public:
    virtual Chunk *child_factory(int chunktype) const;

    void* new_dataobject() const;
    
    int datasize() const { return _datasize; };

    virtual int deserialize(const char *buf);
 private:
    int _datasize;
};

//
// PSF file section classes
// 
class HeaderSection: public SimpleContainer {	
public:
    static const int type = 21;
    HeaderSection() {
	m_chunktype = HeaderSection::type;
    }

    virtual Chunk *child_factory(int chunktype) const;
    
    virtual int deserialize(const char *buf, int abspos);

    const PropertyBlock& get_properties() const { return m_properties; }

    const PSFScalar *get_property(std::string key) const;
private:
    PropertyBlock m_properties;
};

class TypeSection: public IndexedContainer {
public:	
    static const int type = 21;

    virtual Chunk *child_factory(int chunktype) const;

    const DataTypeDef& get_typedef(int id) const { 
	return dynamic_cast<const DataTypeDef &>(get_child(id));
    }

    TypeSection() {
	m_chunktype = TypeSection::type;
    }
};

class TraceSection: public IndexedContainer {
public:	
    static const int type = 21;

    TraceSection(PSFFile *_psf) {
	m_chunktype = TypeSection::type;
	psf = _psf;
    }

    virtual Chunk *child_factory(int chunktype) const;

    NameList get_names() const;

    const DataTypeRef& get_trace_by_index(const TraceIdx &) const;
    const DataTypeRef& get_trace_by_name(std::string name) const;
    const TraceIdx get_traceindex_by_name(std::string name) const;

 private:
    PSFFile *psf;
};

class ZeroPad: public Chunk {
 public:
    static const int type = 20;

    ZeroPad() { m_chunktype = type; }

    int deserialize(const char *buf) {
	const char *startbuf = buf;

	buf += Chunk::deserialize(buf);
	
	uint32_t size = GET_INT32(buf);
	buf += sizeof(uint32_t) + size;
	
	return buf - startbuf;
    }
};	

class SweepSection: public SimpleContainer {	
 public:
    static const int type = 21;

    SweepSection(PSFFile *_psf) {
	m_chunktype = SweepSection::type;
	psf = _psf;
    }

    virtual Chunk *child_factory(int chunktype) const;

    const DataTypeRef & get_sweep(int id) const {
	return dynamic_cast<const DataTypeRef &>(get_child(id));
    }

 private:
    PSFFile *psf;
};

class NonSweepValue : public Chunk {
 public:
    NonSweepValue(PSFFile *_psf) : m_psf(_psf), m_value(NULL) {};
    virtual ~NonSweepValue();

    static const int type = 16;

    const std::string& get_name() const { return m_name.value; }

    const PSFScalar& get_value() const { return *m_value; } 

    const PropertyBlock& get_properties() const { return m_propblock; }

    virtual int deserialize(const char *buf);

    static bool ischunk(int chunktype) {
	return chunktype == type;
    }

 private:
    int m_id;
    PSFStringScalar m_name;
    int m_valuetypeid;
    PSFScalar *m_value;
    PropertyBlock m_propblock;
    PSFFile *m_psf;
};

class SweepValue: public Chunk, public std::vector<PSFVector *> {
public:
    static const int type = 16;

    SweepValue() : m_paramvalues(NULL) { m_chunktype = type; }
    virtual ~SweepValue();

    const std::string& get_name() const { return m_name.value; }

    PSFVector *get_param_values(bool release=false);

    virtual int deserialize(const char *buf, int *n, int windowoffset, PSFFile *psf, Filter &filter) {};

 protected:
    int m_id;
    PSFStringScalar m_name;
    int m_linktypeid;
    PSFVector *m_paramvalues;

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
 SweepValueIterator(T *v, const char *valuebuf, PSFFile *psf, ChildList *filter) : 
    m_v(v), m_psf(psf), m_filter(filter) {
	m_buf = valuebuf;
	m_nextbuf = m_buf + deserialize();
    }
    
 private:
    friend class boost::iterator_core_access;

    void increment() {
	m_buf = m_nextbuf;
	m_nextbuf = m_buf + deserialize();
    }

    T& dereference() const { return *m_v; }	

    bool equal(SweepValueIterator const & other) const { return this->buf == other.buf; }

    int deserialize();
    
    const char *m_buf, *m_nextbuf;
    T *m_v;
    ChildList *m_filter;
    PSFFile *m_psf;
};

class ValueSectionNonSweep: public IndexedContainer {
 public:
    virtual Chunk *child_factory(int chunktype) const;

    ValueSectionNonSweep(PSFFile *_psf) : psf(_psf) {};

    const PSFScalar& get_value(std::string name) const;
    const PropertyBlock &get_value_properties(const std::string name) const;
    
    static const int type = 21;
    PSFFile *psf;
};

class ValueSectionSweep: public Chunk {	
public:
    static const int type = 21;
    
    ValueSectionSweep(PSFFile *psf);

    virtual int deserialize(const char *buf, int abspos);
    
    // Allocate a value of correct class
    SweepValue *new_value() const;

    SweepValue *get_values(Filter &filter) const;
    PSFVector* get_values(std::string name) const;
    PSFVector* get_param_values() const;

    int get_valueoffset(int id) const;
    int get_valuesize() const { return m_valuesize; };

    typedef SweepValueIterator<SweepValue> iterator;
    const iterator begin(SweepValue *, ChildList &filter) const ;
    const iterator end() const;

private:
    void _create_valueoffsetmap(bool windowedsweep);
    
    PSFFile *m_psf;

    int m_valuesize, m_ntraces;
    TraceIDOffsetMap m_offsetmap;
    const char *m_valuebuf, *endbuf;
    
    bool windowedsweep;
};


class PSFFile {	
public:	
    PSFFile(std::string filename);
    ~PSFFile();
    
    NameList get_param_names() const;
    PSFVector *get_param_values() const;
    const PropertyBlock &get_value_properties(std::string name) const;
    PSFVector *get_values(std::string name) const;
    const PSFScalar& get_value(std::string name) const;

    NameList get_names() const;
    
    // Section access functions
    const TypeSection & get_type_section() const { return *m_types; };
    const SweepSection & get_sweep_section() const { return *m_sweeps; };
    const TraceSection & get_trace_section() const { return *m_traces; };
    const ValueSectionSweep & get_value_section_sweep() const { return *m_sweepvalues; };

    // Header properties access functions
    const PropertyBlock& get_header_properties() const { return m_header->get_properties(); }
    
    void open();
    void close();
    
    bool validate() const;
    bool is_done() const;

    std::string m_filename;

private:
    SectionMap load_sections(const char *buf, int size);
    SectionMap load_table_of_contents(const char *buf, int size);
    void deserialize(const char *buf, int size);

    int m_fd;
    const char *m_buffer;
    int m_size;

    HeaderSection *m_header;
    TypeSection *m_types;
    SweepSection *m_sweeps;
    TraceSection *m_traces;
    ValueSectionSweep *m_sweepvalues;
    ValueSectionNonSweep *m_nonsweepvalues;
};

#endif
