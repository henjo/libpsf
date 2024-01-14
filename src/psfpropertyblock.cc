#include <assert.h>

#include "psf.h"
#include "psfdata.h"
#include "psfinternal.h"

PropertyBlock::~PropertyBlock() {
    for(auto&[k,v] : m_propmap){
        delete v;
    }
}

const PSFScalar &PropertyBlock::find(const std::string name) const
{
  PropertyMap::const_iterator found_prop_iter;

  found_prop_iter = m_propmap.find(name);

  if(found_prop_iter == m_propmap.end())
    throw PropertyNotFound();
  else {
    assert(found_prop_iter->second != NULL);
    return *found_prop_iter->second;
  }
}

bool PropertyBlock::hasprop(const std::string name) const
{
  PropertyMap::const_iterator found_prop_iter;

  found_prop_iter = m_propmap.find(name);

  return found_prop_iter != m_propmap.end();
}


const Property & PropertyBlock::findprop(const std::string name) const
{
  PropertyList::const_iterator prop_iter;
  
  for(prop_iter = m_proplist.begin(); prop_iter != m_proplist.end(); prop_iter++)
    if (prop_iter->get_name() == name) 
      break;
  
  return *prop_iter;
}

void PropertyBlock::append_prop(const Property &prop)
{
  // Append property to property list
  m_proplist.push_back(prop); 
  
  // Append property value to property map
  m_propmap[prop.get_name()] = prop.get_value()->clone();
}

int PropertyBlock::deserialize(const char *buf)
{
  const char *startbuf = buf; 

  while(true) {  	
    int chunktype = GET_INT32(buf);

    if(Property::ischunk(chunktype)) {
      Property prop;
      buf += prop.deserialize(buf);
      
      this->append_prop(prop);
    } else
      break;
  }
  return buf - startbuf;
}
