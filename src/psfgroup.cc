#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

int GroupDef::deserialize(const char *buf) {	
    const char *startbuf = buf;

    buf += Chunk::deserialize(buf);
    
    buf += id.deserialize(buf);
    buf += name.deserialize(buf);
    buf += nchildren.deserialize(buf);

    for(int i=0; i < nchildren.value; i++) {
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

PSFData *GroupDef::get_data_object() {
    return new Group(this);
}

PSFData *GroupDef::get_data_object(std::vector<int> *filter) {
    return new Group(this, filter);
}

void GroupDef::_create_valueoffsetmap() {   
    int i=0;
    for(const_iterator iref=begin(); iref != end(); iref++, i++)
	indexmap[(*iref)->get_id()] = i;
}


Group::Group(GroupDef *_groupdef, std::vector<int> *_filter) : groupdef(_groupdef), filter(_filter) {
    resize(groupdef->size());

    int i = 0;
    for(GroupDef::iterator element=groupdef->begin(); element != groupdef->end(); element++, i++) {
	DataTypeRef *typeref = dynamic_cast<DataTypeRef *>(*element);
	PSFDataVector *vec = typeref->get_data_vector();
	(*this)[i] = vec;
    }
}

int Group::deserialize(const char *buf, int n, int windowsize) {
    const char *startbuf = buf;

    std::vector<PSFData *> values;
    std::vector<int> groupdef_index;

    if(filter) 
	for(std::vector<int>::iterator i=filter->begin(); i != filter->end(); i++)
	    values.push_back(groupdef->get_child(*i).get_data_object());	
    else 
	for(GroupDef::iterator element=groupdef->begin(); element != groupdef->end(); element++) 
	    values.push_back((*element)->get_data_object());

    int i=0;
    for(std::vector<PSFData *>::iterator ivalue=values.begin(); ivalue != values.end(); ivalue++, i++) {
	for(int j=0; j < n; j++) {
	    values[i]->deserialize(startbuf + 
				   groupdef->indexmap[(*filter)[i]] * windowsize + 
				   (windowsize - n * (values[i]->datasize())));
	    (*this)[i]->append_value(values[i]);
	}
    }	    
    return groupdef->size() * windowsize;
}
