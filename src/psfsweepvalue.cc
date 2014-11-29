#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

#include <assert.h>

ValueSectionSweep::ValueSectionSweep(PSFFile *psf) : m_psf(psf) {
    m_chunktype = ValueSectionSweep::type;

    windowedsweep = m_psf->get_header_properties().hasprop("PSF window size");

    m_valuebuf = endbuf = NULL;
}

void ValueSectionSweep::_create_valueoffsetmap(bool windowedsweep) {    
  const DataTypeRef *ref = NULL;
  Container::const_iterator itrace;
  int valueoffset = 0;
  int child_datasize;
  
  m_valuesize = 0;
  m_ntraces = 0;
  
  if (windowedsweep) {
    int windowsize = m_psf->get_header_properties().find("PSF window size");
	
    for(itrace = m_psf->get_trace_section().begin(); itrace != m_psf->get_trace_section().end(); itrace++) {
      if(const GroupDef *groupdef = dynamic_cast<const GroupDef *>(*itrace)) 
	child_datasize = groupdef->fill_offsetmap(m_offsetmap, windowsize, valueoffset);
      else
	throw IncorrectChunk((*itrace)->m_chunktype);

      m_valuesize += child_datasize;
      valueoffset += child_datasize;
    }
  } else {
    for(itrace = m_psf->get_trace_section().begin(); itrace != m_psf->get_trace_section().end(); itrace++) {
      ref = dynamic_cast<const DataTypeRef *>(*itrace);

      if(NULL != ref) {
	const DataTypeDef &datatypedef = ref->get_datatype();
	
	// In waveform families there is a data file that only contains sweep values.
	// In this case there are no trace values in the value section, yet there
	// is a trace in the trace section with the same datatypeid as in the sweep.
	// Data types of sweeps have a property called "key" which is set to "sweep" which
	// will be used to detect this problem to get a correct offset and child_datasize
	std::string key = datatypedef.get_properties().find("key").tostring();
	if ("sweep" == key)
	  continue;

	child_datasize = datatypedef.datasize();
	m_offsetmap[ref->get_id()] = valueoffset + 8;
      } else if(const GroupDef *groupdef = dynamic_cast<const GroupDef *>(*itrace))
	child_datasize = groupdef->fill_offsetmap(m_offsetmap, 0, valueoffset + 8);
      else 
	throw IncorrectChunk((*itrace)->m_chunktype);
      
      m_valuesize += 8 + child_datasize;
      valueoffset += 8 + child_datasize;
    }
  }
}

SweepValue* ValueSectionSweep::get_values(Filter &filter) const {
    const char *buf = m_valuebuf;

    SweepValue *value = new_value();
    
    int n = m_psf->get_header_properties().find("PSF sweep points");
    
    int windowoffset = 0;
    value->deserialize(buf, &n, windowoffset, m_psf, filter);

    return value;
}

PSFVector* ValueSectionSweep::get_values(std::string name) const {
    // Create filter for retrieving the trace with correct name
    Filter filter;
    filter.push_back(&m_psf->get_trace_section().get_trace_by_name(name));
    
    SweepValue *v = get_values(filter);
    
    PSFVector *result = v->at(0);
    
    // Clear vector to avoid deallocation of result
    v->clear();

    delete v;

    return result;
}


PSFVector* ValueSectionSweep::get_param_values() const {
    Filter filter;
    SweepValue *v = get_values(filter);

    PSFVector *result = v->get_param_values(true);

    delete v;

    return result;
}


int ValueSectionSweep::deserialize(const char *buf, int abspos) {
    const char *startbuf = buf;

    buf += Chunk::deserialize(buf);
	
    uint32_t endpos = GET_INT32(buf);
    buf += sizeof(uint32_t);

    if (windowedsweep) {
	ZeroPad pad;
	buf += pad.deserialize(buf);
    }  
        
    _create_valueoffsetmap(windowedsweep);

    m_valuebuf = buf;

    endbuf = startbuf + endpos - abspos;

    return endpos-abspos;
};


int ValueSectionSweep::get_valueoffset(int id) const {
    return m_offsetmap.find(id)->second;
}
    
const ValueSectionSweep::iterator ValueSectionSweep::begin(SweepValue *value, ChildList &filter) const {
    return iterator(value, m_valuebuf, m_psf, &filter);
}    

const ValueSectionSweep::iterator ValueSectionSweep::end() const {
    return iterator(NULL, endbuf-4, NULL, NULL);
}    

SweepValue * ValueSectionSweep::new_value() const {
    if(windowedsweep)
	return new SweepValueWindowed();
    else
	return new SweepValueSimple();
}

PSFVector * SweepValue::get_param_values(bool release) {
    PSFVector *result = m_paramvalues;

    if(release)
	m_paramvalues = NULL;

    return result;	
}

SweepValue::~SweepValue() {
    if (m_paramvalues)
	delete(m_paramvalues);

    for(std::vector<PSFVector *>::const_iterator i = begin(); i != end(); i++)
	delete(*i);
}

int SweepValueWindowed::deserialize(const char *buf, int *totaln, int windowoffset, PSFFile *psf, 
				    Filter &filter) {
    const char *startbuf = buf;

    int windowsize = psf->get_header_properties().find("PSF window size");
    int ntraces    = psf->get_header_properties().find("PSF traces");

    // Create parameter vector
    DataTypeRef &paramtype = *((DataTypeRef *)psf->get_sweep_section()[0]);
    if(m_paramvalues == NULL)
	m_paramvalues = paramtype.new_vector();

    // Create data vectors
    clear();
    for(ChildList::const_iterator j=filter.begin(); j != filter.end(); j++) {
	DataTypeRef *trace = (DataTypeRef *) *j;
	PSFVector *vec = trace->new_vector();
	vec->resize(*totaln);
	push_back(vec);
    }

    for(int i=0; i < *totaln; ) {
	buf += Chunk::deserialize(buf);
    
	int tmp = GET_INT32(buf);
	int windowleft = tmp >> 16; 
	int n = tmp & 0xffff;       // Number of data points in window

	buf += 4;
	windowoffset += 4;
    
	// Deserialize parameter values from file to parameter vector (m_paramvalues)
	int pwinstart = m_paramvalues->size();
	m_paramvalues->resize(m_paramvalues->size() + n);
	for(int j=0; j < n; j++)
	    buf += paramtype.deserialize_data(m_paramvalues->ptr_at(pwinstart + j), buf);

	const char *valuebuf = buf;        // Save start of trace values pointer in buffer
	const_iterator idatavec = begin(); // Init iterator of destination trace vectors
	for(Filter::const_iterator j=filter.begin(); j != filter.end(); j++, idatavec++) {
	    const DataTypeRef &typeref = dynamic_cast<const DataTypeRef &>(**j);

	    // calculate buffer pointer
	    buf = valuebuf +  psf->get_value_section_sweep().get_valueoffset((*j)->get_id()) +
		(windowsize - n * typeref.datasize());
	    
	    for(int k=0; k < n; k++)
		buf += typeref.deserialize_data((*idatavec)->ptr_at(i+k), buf);
	}

	// Advance buffer pointer to end of trace values
	buf = valuebuf + ntraces * windowsize;
	i += n;
    }
    return buf - startbuf;
}

int SweepValueSimple::deserialize(const char *buf, int *n, int windowoffset, PSFFile *psf, Filter &filter) {
    const char *startbuf = buf;
    const ValueSectionSweep &valuesection = psf->get_value_section_sweep();

    // Create data vectors
    for(ChildList::const_iterator j=filter.begin(); j != filter.end(); j++) {
	DataTypeRef *trace = (DataTypeRef *) *j;
	PSFVector *vec = trace->new_vector();
	vec->resize(*n);
	push_back(vec);
    }

    DataTypeRef &paramtype = *((DataTypeRef *)psf->get_sweep_section()[0]);

    if(m_paramvalues == NULL)
	m_paramvalues = paramtype.new_vector();

    int paramstartidx = m_paramvalues->size();

    m_paramvalues->resize(paramstartidx + *n);

    for(int i=0; i < *n; i++) {
	const char *pointstartbuf = buf;

	buf += Chunk::deserialize(buf);
    
	int paramtypeid = GET_INT32(buf);
	buf += 4;
    
	assert(paramtypeid == paramtype.get_id());

	buf += paramtype.deserialize_data(m_paramvalues->ptr_at(paramstartidx + i), buf);

	const char *valuebuf = buf;

	int k=0;
	for(Filter::const_iterator j=filter.begin(); j != filter.end(); j++, k++) {
	    const DataTypeRef *trace = dynamic_cast<const DataTypeRef *>(*j);

	    buf = valuebuf + valuesection.get_valueoffset(trace->get_id());

	    trace->deserialize_data(this->at(k)->ptr_at(i), buf);
	}
	buf = valuebuf + valuesection.get_valuesize();
    }

    return buf - startbuf;
}


template<class T>
int SweepValueIterator<T>::deserialize() {
    if((m_buf == NULL) || (m_psf == NULL) || (m_filter == NULL))
	return 0;
    
    int chunkid = GET_INT32(m_buf);

    int n = 1;
    int windowoffset = 0;

    if(chunkid != CHUNKID_VALUESECTIONEND)
	return m_v->deserialize(m_buf, &n, windowoffset, m_psf, *m_filter);
    else
	return 0;
}

