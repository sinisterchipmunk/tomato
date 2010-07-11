#include "tomato.h"

Handle<Value> find_or_create_object_chain(V8Tomato *tomato, VALUE chain)
{
  Handle<Value> value = tomato_v8_context(tomato)->Global();
  if (value->IsObject())
  {
    Handle<Object> object = Handle<Object>::Cast(value);
    if (!NIL_P(chain))
    {
      int len = RARRAY_LEN(chain);
      VALUE *items = RARRAY_PTR(chain);
      
      for (int i = 0; i < len; i++)
      {
        Handle<String> object_name = String::New(StringValuePtr(items[i]));
        value = object->Get(object_name);
        if (value->IsObject())
        {
          object = Handle<Object>::Cast(value);  
        }
        else
        {
          Handle<Object> new_object = Object::New();
          object->Set(object_name, new_object);
          object = new_object;
        }
      }
    }
    return object;
  }
  return value;
}
