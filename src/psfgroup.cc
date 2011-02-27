#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

int GroupDef::deserialize(const char *buf) {	
    const char *startbuf = buf;

    buf += Chunk::deserialize(buf);
    
    id = GET_INT32(buf); buf+=4;

    buf += name.deserialize(buf);

    nchildren = GET_INT32(buf); buf+=4;

    for(int i=0; i < nchildren; i++) {
	Chunk *chunk = deserialize_child(&buf);  
	add_child(chunk);
	namemap[chunk->get_name()] = i;
    }

    _create_valueindexmap();
    
    return buf - startbuf;
}
    
Chunk* GroupDef::child_factory(int chunktype) const {
    if(DataTypeRef::ischunk(chunktype))
	return new DataTypeRef(psf);
    else
	throw IncorrectChunk(chunktype);
}	

/*
void* GroupDef::new_dataobject() const {
    return new Group(this);
}

Group* GroupDef::new_group(std::vector<int> *filter) const {
    return new Group(this, filter);
}
*/

int GroupDef::fill_offsetmap(TraceIDOffsetMap& offsetmap, int windowsize, int startoffset) const {   
    int offset = startoffset;

    for(const_iterator iref=begin(); iref != end(); iref++) {
	offsetmap[(*iref)->get_id()] = offset;
	if(windowsize) 
	    offset += windowsize;
	else {
	    const DataTypeRef &typeref = dynamic_cast<const DataTypeRef &>(**iref);
	    offset += typeref.datasize();
	}
    }
    return offset - startoffset;
}

void GroupDef::_create_valueindexmap() {   
    int i=0;
    for(const_iterator iref=begin(); iref != end(); iref++, i++)
	indexmap[(*iref)->get_id()] = i;
}

const Chunk & GroupDef::get_child(std::string name) const {
    return *at(namemap.find(name)->second);
}

int GroupDef::get_child_index(std::string name) const {
    return namemap.find(name)->second;
}

/*
Group::Group(const GroupDef *_groupdef, std::vector<int> *_filter) : groupdef(_groupdef), filter(_filter) {
    if(filter) {
	resize(filter->size());	
	
	int i=0;
	for(std::vector<int>::iterator itypeid=filter->begin(); itypeid != filter->end(); itypeid++, i++) {
	    const DataTypeRef &typeref = dynamic_cast<const DataTypeRef &>(groupdef->get_child(*itypeid));
	    at(i) = typeref.new_vector();
	}
    } else {
	resize(groupdef->size());

	int i = 0;
	for(GroupDef::const_iterator element=groupdef->begin(); element != groupdef->end(); element++, i++) {
	    const DataTypeRef *typeref = dynamic_cast<const DataTypeRef *>(*element);
	    PSFVector *vec = typeref->new_vector();
	    at(i) = vec;
	}
    }
}

Group::~Group() {
    for(const_iterator i=begin(); i != end(); i++)
	delete(*i);
}

int Group::deserialize(const char *buf, int n, int windowsize) {
    const char *startbuf = buf;

    if(filter) {
	int i=0;
	for(std::vector<int>::iterator itypeid=filter->begin(); itypeid != filter->end(); itypeid++, i++) { 
	    const DataTypeRef &typeref = dynamic_cast<const DataTypeRef &>(groupdef->get_child(*itypeid));
	    const DataTypeDef &type_def = typeref.get_datatype();
	    PSFVector *vec = at(i);

	    int offset = vec->size();
	    vec->resize(offset + n);
	    
	    buf = startbuf +  groupdef->indexmap.find(*itypeid)->second * windowsize + 
		(windowsize - n * type_def.datasize());
	    for(int j=0; j < n; j++) {
		buf += type_def.deserialize_data(vec->ptr_at(offset+j), buf);
	    }
	}
    } else {
	int i = 0;
	for(GroupDef::const_iterator element=groupdef->begin(); element != groupdef->end(); element++, i++) {
	    const DataTypeRef *typeref = dynamic_cast<const DataTypeRef *>(*element);
	    const DataTypeDef &type_def = typeref->get_datatype();
	    PSFVector *vec = at(i);
	    for(int j=0; j < n; j++) {
		buf = startbuf +  groupdef->indexmap.find((*element)->get_id())->second * windowsize + 
		    (windowsize - n * type_def.datasize());
		type_def.deserialize_data(vec->ptr_at(j), buf);
	    }
	}
	
    }
	
    return groupdef->size() * windowsize;
}
*/
