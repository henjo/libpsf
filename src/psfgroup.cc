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
    }

    _create_valueoffsetmap();
    
    return buf - startbuf;
}
    
Chunk* GroupDef::child_factory(int chunktype) {
    if(DataTypeRef::ischunk(chunktype))
	return new DataTypeRef(psf);
    else
	throw IncorrectChunk(chunktype);
}	

void* GroupDef::new_dataobject() {
    return new Group(this);
}

Group* GroupDef::new_group(std::vector<int> *filter) {
    return new Group(this, filter);
}

void GroupDef::_create_valueoffsetmap() {   
    int i=0;
    for(const_iterator iref=begin(); iref != end(); iref++, i++)
	indexmap[(*iref)->get_id()] = i;
}


Group::Group(GroupDef *_groupdef, std::vector<int> *_filter) : groupdef(_groupdef), filter(_filter) {
    if(filter) {
	resize(filter->size());	
	
	int i=0;
	for(std::vector<int>::iterator itypeid=filter->begin(); itypeid != filter->end(); itypeid++, i++) {
	    DataTypeRef &typeref = dynamic_cast<DataTypeRef &>(groupdef->get_child(*itypeid));
	    at(i) = typeref.new_vector();
	}
    } else {
	resize(groupdef->size());

	int i = 0;
	for(GroupDef::iterator element=groupdef->begin(); element != groupdef->end(); element++, i++) {
	    DataTypeRef *typeref = dynamic_cast<DataTypeRef *>(*element);
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
	    DataTypeRef &typeref = dynamic_cast<DataTypeRef &>(groupdef->get_child(*itypeid));
	    DataTypeDef &type_def = typeref.get_datatype();
	    PSFVector *vec = at(i);

	    int offset = vec->size();
	    vec->resize(offset + n);
	    
	    for(int j=0; j < n; j++) {
		buf = startbuf +  groupdef->indexmap[*itypeid] * windowsize + 
		    (windowsize - n * type_def.datasize());
		type_def.deserialize_data(vec->ptr_at(offset+j), buf);
	    }
	}
    } else {
	int i = 0;
	for(GroupDef::iterator element=groupdef->begin(); element != groupdef->end(); element++, i++) {
	    DataTypeRef *typeref = dynamic_cast<DataTypeRef *>(*element);
	    DataTypeDef &type_def = typeref->get_datatype();
	    PSFVector *vec = at(i);
	    for(int j=0; j < n; j++) {
		buf = startbuf +  groupdef->indexmap[(*element)->get_id()] * windowsize + 
		    (windowsize - n * type_def.datasize());
		type_def.deserialize_data(vec->ptr_at(j), buf);
	    }
	}
	
    }
	
    return groupdef->size() * windowsize;
}
