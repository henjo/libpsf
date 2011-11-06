#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

#include <assert.h>

ValueSectionSweep::ValueSectionSweep(PSFFile *_psf) : psf(_psf) {
    chunktype = ValueSectionSweep::type;

    windowedsweep = (psf->header->get_property("PSF window size") != NULL);

    valuebuf = endbuf = NULL;
}

void ValueSectionSweep::_create_valueoffsetmap(bool windowedsweep) {    
    _valuesize = 0;

    int valueoffset = 0;
    int child_datasize;

    if (windowedsweep) {
	int windowsize = *psf->header->get_property("PSF window size");
	
	for(Container::const_iterator trace=psf->traces->begin(); trace != psf->traces->end(); trace++) {
	    if(const GroupDef *groupdef = dynamic_cast<const GroupDef *>(*trace)) 
		child_datasize = groupdef->fill_offsetmap(offsetmap, windowsize, valueoffset);
	    else
		throw IncorrectChunk((*trace)->chunktype);

	    _valuesize += child_datasize;
	    valueoffset += child_datasize;
	}
    } else {
	for(Container::const_iterator trace=psf->traces->begin(); trace != psf->traces->end(); trace++) {
	    if(const DataTypeRef *ref = dynamic_cast<const DataTypeRef *>(*trace)) {
		child_datasize = ref->get_datatype().datasize();
		offsetmap[ref->get_id()] = valueoffset + 8;
	    } else if(const GroupDef *groupdef = dynamic_cast<const GroupDef *>(*trace))
		child_datasize = groupdef->fill_offsetmap(offsetmap, 0, valueoffset + 8);
	    else 
		throw IncorrectChunk((*trace)->chunktype);

	    _valuesize += child_datasize;
	    valueoffset += 8 + child_datasize;
	}
    }
}

SweepValue* ValueSectionSweep::get_values(Filter &filter) const {
    const char *buf = valuebuf;

    SweepValue *value = new_value();
    
    int n = *psf->header->get_property("PSF sweep points");
    
    int windowoffset = 0;
    value->deserialize(buf, &n, windowoffset, psf, filter);

    return value;
}

PSFVector* ValueSectionSweep::get_values(std::string name) const {
    // Create filter for retrieving the trace with correct name
    Filter filter;
    filter.push_back(&psf->traces->get_trace_by_name(name));
    
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

    valuebuf = buf;

    endbuf = startbuf + endpos - abspos;

    return endpos-abspos;
};


int ValueSectionSweep::valueoffset(int id) const {
    return offsetmap.find(id)->second;
}
    
const ValueSectionSweep::iterator ValueSectionSweep::begin(SweepValue *value, ChildList &filter) const {
    return iterator(value, valuebuf, psf, &filter);
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
    PSFVector *result = paramvalues;

    if(release)
	paramvalues = NULL;

    return result;	
}

SweepValue::~SweepValue() {
    if (paramvalues)
	delete(paramvalues);

    for(std::vector<PSFVector *>::const_iterator i = begin(); i != end(); i++)
	delete(*i);
}

int SweepValueWindowed::deserialize(const char *buf, int *totaln, int windowoffset, PSFFile *psf, 
				    Filter &filter) {
    const char *startbuf = buf;

    int windowsize = *psf->header->get_property("PSF window size");

    // Create parameter vector
    DataTypeRef &paramtype = *((DataTypeRef *)(*psf->sweeps)[0]);
    if(paramvalues == NULL)
	paramvalues = paramtype.new_vector();

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
	int n = tmp & 0xffff;

	buf += 4;
	windowoffset += 4;
    
	int pwinstart = paramvalues->size();
	paramvalues->resize(paramvalues->size() + n);
	for(int j=0; j < n; j++)
	    buf += paramtype.deserialize_data(paramvalues->ptr_at(pwinstart + j), buf);

	const char *valuebuf = buf;
	
	const_iterator idatavec = begin();

	for(Filter::const_iterator j=filter.begin(); j != filter.end(); j++, idatavec++) {
	    const DataTypeRef &typeref = dynamic_cast<const DataTypeRef &>(**j);

	    // calculate buffer pointer
	    buf = valuebuf +  psf->sweepvalues->valueoffset((*j)->get_id()) +
		(windowsize - n * typeref.datasize());
	    
	    for(int k=0; k < n; k++)
		buf += typeref.deserialize_data((*idatavec)->ptr_at(i+k), buf);
	}
	
	i += n;
    }
    return buf - startbuf;
}

int SweepValueSimple::deserialize(const char *buf, int *n, int windowoffset, PSFFile *psf, Filter &filter) {
    const char *startbuf = buf;

    // Create data vectors
    for(ChildList::const_iterator j=filter.begin(); j != filter.end(); j++) {
	DataTypeRef *trace = (DataTypeRef *) *j;
	PSFVector *vec = trace->new_vector();
	vec->resize(*n);
	push_back(vec);
    }

    DataTypeRef &paramtype = *((DataTypeRef *)(*psf->sweeps)[0]);

    if(paramvalues == NULL)
	paramvalues = paramtype.new_vector();

    int paramstartidx = paramvalues->size();

    paramvalues->resize(paramstartidx + *n);

    for(int i=0; i < *n; i++) {
	const char *pointstartbuf = buf;

	buf += Chunk::deserialize(buf);
    
	int paramtypeid = GET_INT32(buf);
	buf += 4;
    
	assert(paramtypeid == paramtype.get_id());

	buf += paramtype.deserialize_data(paramvalues->ptr_at(paramstartidx + i), buf);

	const char *valuebuf = buf;

	int k=0;
	for(Filter::const_iterator j=filter.begin(); j != filter.end(); j++, k++) {
	    const DataTypeRef *trace = dynamic_cast<const DataTypeRef *>(*j);

	    buf = valuebuf + psf->sweepvalues->valueoffset(trace->get_id());

	    trace->deserialize_data(this->at(k)->ptr_at(i), buf);
	}
	buf = valuebuf + 8 * psf->traces->size() + psf->sweepvalues->valuesize();
    }

    return buf - startbuf;
}


template<class T>
int SweepValueIterator<T>::deserialize() {
    if((buf == NULL) || (psf == NULL) || (filter == NULL))
	return 0;
    
    int chunkid = GET_INT32(buf);

    int n = 1;
    int windowoffset = 0;

    if(chunkid != CHUNKID_VALUESECTIONEND)
	return v->deserialize(buf, &n, windowoffset, psf, *filter);
    else
	return 0;
}

