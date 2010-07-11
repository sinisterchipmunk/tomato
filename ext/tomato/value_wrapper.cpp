#include "tomato.h"

static void free_value_wrapper(Persistent<Value> object, void* parameter)
{
  printf("cleaning\n");
  ValueWrapper *wrapper = (ValueWrapper *)parameter;
  pop_rb_reference(wrapper->tomato, wrapper->value);
  delete wrapper;
}

void register_value_wrapper(Handle<Object> target, V8Tomato *tomato, VALUE value)
{
  push_rb_reference(tomato, value);
  ValueWrapper *wrapper = new ValueWrapper;
  wrapper->tomato = tomato;
  wrapper->value = value;
  target->Set(String::New("_tomato_wrapper"), External::New(wrapper));

  Persistent<Value> pval = Persistent<Value>::New(target);
  pval.MakeWeak(wrapper, &free_value_wrapper);
}

ValueWrapper *extract_value_wrapper(Handle<Object> target)
{
  Local<Value> v8_wrapper       = target->Get(String::New("_tomato_wrapper"));
  if (!v8_wrapper->IsExternal())  throw std::string("Error: _tomato_wrapper is not an object (BUG: please report)");
  ValueWrapper *wrapper = (ValueWrapper *)Local<External>::Cast(v8_wrapper)->Value();
}
