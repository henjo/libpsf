#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

#include <assert.h>

int ValueSectionSweep::valueoffset(int id) {
    return valueoffsetmap[id];
}
    
const ValueSectionSweep::iterator ValueSectionSweep::begin(SweepValue *value, ChildList &filter) const {
    return iterator(value, valuebuf, psf, &filter);
}    

const ValueSectionSweep::iterator ValueSectionSweep::end() const {
    return iterator(NULL, endbuf-4, NULL, NULL);
}    

SweepValue * ValueSectionSweep::new_value() {
    if(windowedsweep)
	return new SweepValueWindowed();
    else
	return new SweepValueSimple();
}

int HeaderSection::deserialize(const char *buf, int abspos) {
    int n = SimpleContainer::deserialize(buf, abspos);

    for(Container::const_iterator child=begin(); child !=end(); child++) {
	Property *prop = (Property *)*child;
	properties[prop->get_name()] = prop->get_value();
    }

    return n;
}

int SweepValueWindowed::deserialize(const char *buf, int *totaln, int windowoffset, PSFFile *psf, 
				    ChildList &filter) {
    const char *startbuf = buf;

    clear();

    std::vector<Group *> groups;

    std::vector<int> filter2;
    for(ChildList::iterator i=filter.begin(); i!=filter.end(); i++)
	filter2.push_back((*i)->get_id());
    
    // Create group and copy pointers to data vectors from group to self
    int k = 0;
    for(TraceSection::const_iterator j=psf->traces->begin(); j != psf->traces->end(); j++) {
	GroupDef *groupdef = dynamic_cast<GroupDef *>(*j);

	resize(size() + groupdef->size());
    
	Group *group = dynamic_cast<Group *>(groupdef->get_data_object(&filter2));

	groups.push_back(group);

	for(Group::iterator ivec=group->begin(); ivec != group->end(); ivec++) {
	    (*this)[k++] = *ivec;
	}
    }

    for(int i=0; i < *totaln; ) {
	buf += Chunk::deserialize(buf);
    
	int tmp = GET_INT32(buf);
	int windowleft = tmp >> 16;
	int n = tmp & 0xffff;

	buf += 4;
	windowoffset += 4;
    
	paramtype = *((DataTypeRef *)(*psf->sweeps)[0]);

	if(paramvalues == NULL)
	    paramvalues = paramtype.get_data_vector();

	PSFData *paramvalue = paramtype.get_data_object();

	for(int j=0; j < n; j++) {
	    buf += paramvalue->deserialize(buf);
	    paramvalues->append_value(paramvalue);
	}

	const char *valuebuf = buf;

	int windowsize = *psf->header->get_property("PSF window size");

	for(std::vector<Group *>::iterator igroup = groups.begin(); igroup != groups.end(); igroup++)
	    buf += (*igroup)->deserialize(buf, n, windowsize);
	
	i += n;
    }
    return buf - startbuf;
}

int SweepValueSimple::deserialize(const char *buf, int *n, int windowoffset, PSFFile *psf, ChildList &filter) {
    const char *startbuf = buf;

    for(int i=0; i < *n; i++) {
	const char *pointstartbuf = buf;

	buf += Chunk::deserialize(buf);
    
	int paramtypeid = GET_INT32(buf);
	buf += 4;
    
	paramtype = psf->sweeps->get_sweep(paramtypeid);

	if(paramvalues == NULL)
	    paramvalues = paramtype.get_data_vector();

	PSFData *paramvalue = paramtype.get_data_object();
	buf += paramvalue->deserialize(buf);

	paramvalues->append_value(paramvalue);

	const char *valuebuf = buf;

	clear();
	for(ChildList::const_iterator j=filter.begin(); j != filter.end(); j++) {
	    DataTypeRef *trace = (DataTypeRef *) *j;

	    buf = valuebuf + psf->sweepvalues->valueoffset(trace->get_id());

	    // Uncertain what this is, the value is 16 or 17
	    int tracetype = GET_INT32(buf);
	
	    int traceid = GET_INT32(buf+4);
	
	    buf += 8;
	
	    assert(trace->get_id() == traceid);

	    PSFData *value = trace->get_data_object();
	    value->deserialize(buf);
	    //	    push_back(value);
	}
	buf = pointstartbuf + (valuebuf-pointstartbuf) + 8 * psf->traces->size() + psf->sweepvalues->valuesize();
    }

    return buf - startbuf;
}


ValueSectionSweep::ValueSectionSweep(PSFFile *_psf) : psf(_psf) {
    chunktype = ValueSectionSweep::type;

    windowedsweep = (psf->header->get_property("PSF window size") != NULL);

    valuebuf = endbuf = NULL;
}

void ValueSectionSweep::_create_valueoffsetmap(bool windowedsweep) {    
    _valuesize = 0;
    int valueoffset = 0;

    for(Container::const_iterator trace=psf->traces->begin(); trace != psf->traces->end(); trace++) {
	if(DataTypeRef *ref = dynamic_cast<DataTypeRef *>(*trace)) {
	    int child_datasize = ref->get_datatype().datasize();

	    _valuesize += child_datasize;

	    valueoffsetmap[(*trace)->get_id()] = valueoffset;
	    valueoffset += 8 + child_datasize;
	}
    }
}



SweepValue* ValueSectionSweep::get_values(ChildList &filter) {
    const char *buf = valuebuf;

    SweepValue *value = new_value();
    
    int n = *psf->header->get_property("PSF sweep points");
    
    int windowoffset = 0;
    value->deserialize(buf, &n, windowoffset, psf, filter);

    return value;
}

PSFDataVector* ValueSectionSweep::get_values(std::string name) {
    // Create filter for retrieving the trace with correct name
    ChildList filter;
    Chunk &trace = psf->traces->get_trace_by_name(name);
    filter.push_back(&trace);
    
    SweepValue *v = get_values(filter);
    
    return (*v)[0];
}


PSFDataVector* ValueSectionSweep::get_param_values() {
    ChildList filter;
    SweepValue *v = get_values(filter);
    return v->get_param_values();
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

