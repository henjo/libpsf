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
