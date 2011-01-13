#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

Chunk * TraceSection::child_factory(int chunktype) {
    if(DataTypeRef::ischunk(chunktype))
	return new DataTypeRef(psf);
    else if(GroupDef::ischunk(chunktype))
	return new GroupDef(psf);
    else {
	std::cerr << "Unexpected chunktype: " << chunktype << std::endl;
	throw IncorrectChunk(chunktype);
    }
}

NameList TraceSection::get_names() {
    NameList result;

    for(const_iterator i=begin(); i != end(); i++) {
	GroupDef *group = dynamic_cast<GroupDef *>(*i);
	if(group) {
	    NameList grouptracenames = group->get_names();
	    result.reserve(result.size() + distance(grouptracenames.begin(), grouptracenames.end()));
	    result.insert(result.end(), grouptracenames.begin(), grouptracenames.end());
	} else
	    result.push_back((*i)->get_name());
    }
    return result;
}

DataTypeRef & TraceSection::get_trace_by_name(const std::string name) {
    for(const_iterator i=begin(); i != end(); i++) {
	GroupDef *groupdef = dynamic_cast<GroupDef *>(*i);
	if(groupdef) {
	    return dynamic_cast<DataTypeRef &>(groupdef->get_child(name));
	} else
	    return dynamic_cast<DataTypeRef &>(Container::get_child(name));
    }
}
