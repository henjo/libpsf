#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

Chunk * TraceSection::child_factory(int chunktype) const {
    if(DataTypeRef::ischunk(chunktype))
	return new DataTypeRef(psf);
    else if(GroupDef::ischunk(chunktype))
	return new GroupDef(psf);
    else {
	std::cerr << "Unexpected chunktype: " << chunktype << std::endl;
	throw IncorrectChunk(chunktype);
    }
}

NameList TraceSection::get_names() const {
    NameList result;

    for(const_iterator i=begin(); i != end(); i++) {
	const GroupDef *groupdef = dynamic_cast<const GroupDef *>(*i);
	if(groupdef) {
	    NameList grouptracenames = groupdef->get_names();
	    result.reserve(result.size() + distance(grouptracenames.begin(), grouptracenames.end()));
	    result.insert(result.end(), grouptracenames.begin(), grouptracenames.end());
	} else
	    result.push_back((*i)->get_name());
    }
    return result;
}

const DataTypeRef & TraceSection::get_trace_by_name(const std::string name) const {
    for(const_iterator i=begin(); i != end(); i++) {
	const GroupDef *groupdef = dynamic_cast<const GroupDef *>(*i);
	if(groupdef) {
	    return dynamic_cast<const DataTypeRef &>(groupdef->get_child(name));
	} else
	    return dynamic_cast<const DataTypeRef &>(get_child(name));
    }
}

const TraceIdx TraceSection::get_traceindex_by_name(const std::string name) const {
    TraceIdx index;
    
    // Try to find trace at root level
    int i = get_child_index(name);

    // If not found, try to search groups
    if(i == -1) {
	int groupindex=0;
	for(const_iterator i=begin(); i != end(); i++, groupindex++) {
	    const GroupDef *groupdef = dynamic_cast<const GroupDef *>(*i);
	    if(groupdef) {
		index.push_back(groupindex);
		index.push_back(groupdef->get_child_index(name));
	    } 
	}
    } else
	index.push_back(i);

    return index;
}


const DataTypeRef& TraceSection::get_trace_by_index(const TraceIdx &idx) const {
    if(idx.size() == 1) {    
	const DataTypeRef& trace = dynamic_cast<const DataTypeRef&>(*at(idx[0]));
	return trace;
    } else {
	TraceIdx newindex(idx);

	const GroupDef& groupdef = dynamic_cast<const GroupDef &>(*at(idx[0]));

	return dynamic_cast<const DataTypeRef&>(*groupdef.at(idx[1]));
    }
}
