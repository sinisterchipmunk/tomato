#include "tomato.h"

static VALUE ruby_array_from(V8Tomato *tomato, Handle<Array> result);
static VALUE ruby_numeric_from(const Handle<Value> &number);
static VALUE ruby_date_from(const Handle<Value> &date);
static VALUE ruby_string_from(const Handle<Value> &value);
static VALUE ruby_symbol_from(const Handle<Object> &value);
static VALUE ruby_object_from(V8Tomato *tomato, Handle<Value> result);
static VALUE ruby_hash_from(V8Tomato *tomato, const Handle<Object> &object);

VALUE ruby_value_of(V8Tomato *tomato, Handle<Value> result)
{
  if (result->IsUndefined()) { return ID2SYM(rb_intern("undefined")); }
  if (result->IsNull())      { return Qnil; }
  if (result->IsBoolean())
  {
    if (result->IsTrue())       { return Qtrue; }
    else if (result->IsFalse()) { return Qfalse; }
  }
  if (result->IsString())    { return ruby_string_from(result); }
  if (result->IsFunction())  { return ruby_string_from(result); }
  if (result->IsArray())     { Handle<Array> array = Handle<Array>::Cast(result); return ruby_array_from(tomato, array); }
  if (result->IsNumber())    { return ruby_numeric_from(result); }
  if (result->IsDate())      { return ruby_date_from(result); }
  /* TODO: RegExp support: To patch or not to patch? */    
  if (result->IsObject())    { return ruby_object_from(tomato, result); }
  
  return Qnil;
}


/* First checks for any special cases set up internally (i.e. Hash). If all else fails, returns the JSON for this
   object. */
static VALUE ruby_object_from(V8Tomato *tomato, Handle<Value> result)
{
  if (result->IsObject())
  {
    Handle<Object> object = Handle<Object>::Cast(result);
    
    if (object->Get(String::New("_tomato_hash"))->IsTrue())         return ruby_hash_from(tomato, object);
    if (object->Get(String::New("_tomato_symbol"))->IsTrue())       return ruby_symbol_from(object);
    try {
      ValueWrapper *wrapper = extract_value_wrapper(object);
      return wrapper->value;
    } catch(std::string)
    {
      /* it's not a wrapped object. Doing nothing will fall back to JSON representation. */
    }
  }
  
  /* Call Javascript's JSON.stringify(object) method. If that can't be done for any reason, return nil. */
  Handle<Value> json = tomato_v8_context(tomato)->Global()->Get(String::New("JSON"));
  if (json->IsObject())
  {
    Handle<Value> stringify = Handle<Object>::Cast(json)->Get(String::New("stringify"));
    if (stringify->IsFunction())
    {
      String::Utf8Value str(Handle<Function>::Cast(stringify)->Call(
        Handle<Object>::Cast(json),
        1, 
        &result
      ));
      /* TODO translate the result to Ruby! */
      return rb_str_new2(ToCString(str));
    }
  }
  return ID2SYM(rb_intern("unknown"));
}

static VALUE ruby_hash_from(V8Tomato *tomato, const Handle<Object> &object)
{
  VALUE hash = rb_hash_new();
  
  Handle<Array> keys = Handle<Array>::Cast(object->Get(String::New("_tomato_hash_keys")));
  Handle<Array> values = Handle<Array>::Cast(object->Get(String::New("_tomato_hash_values")));
  
  int length = keys->Length();
  
  for (int i = 0; i < length; i++)
  {
    rb_hash_aset(hash, ruby_value_of(tomato, keys->Get(i)), ruby_value_of(tomato, values->Get(i)));
  }

  return hash;
}

static VALUE ruby_symbol_from(const Handle<Object> &value)
{
  String::Utf8Value symbol_value(value->Get(String::New("symbol")));
  return ID2SYM(rb_intern(ToCString(symbol_value)));
}

static VALUE ruby_string_from(const Handle<Value> &value)
{
  String::Utf8Value string_value(value);
  return rb_str_new2(ToCString(string_value));
}

static VALUE ruby_date_from(const Handle<Value> &value)
{
  const Handle<Date> date = Handle<Date>::Cast(value);
  return rb_funcall(rb_cTime, rb_intern("at"), 1, DBL2NUM(date->NumberValue() / 1000.0));
}

static VALUE ruby_array_from(V8Tomato *tomato, Handle<Array> array)
{
  unsigned int length = array->Length();
  VALUE rbarr = rb_ary_new2(length);
  for (unsigned int i = 0; i < length; i++)
    rb_ary_push(rbarr, ruby_value_of(tomato, array->Get(Integer::New(i))));
  return rbarr;
}

static VALUE ruby_numeric_from(const Handle<Value> &number)
{
  if (number->IsInt32())  { return INT2FIX(number->Int32Value()); }
  if (number->IsUint32()) { return INT2FIX(number->Uint32Value()); }
  /* TODO FIXME: There's no way to know if we're facing a Uint64. It works as a Float but user may not expect a Float... */
  //if (number->IsInt64())  { return INT2NUM(number->IntegerValue()); }
  return DBL2NUM(number->NumberValue());
}

Handle<Value> inspect_js(V8Tomato *tomato, Handle<Value> obj)
{
  /* Call Javascript's JSON.stringify(object) method. If that can't be done for any reason, return an error. */
  Handle<Value> json = tomato_v8_context(tomato)->Global()->Get(String::New("JSON"));
  if (json->IsObject())
  {
    Handle<Value> stringify = Handle<Object>::Cast(json)->Get(String::New("stringify"));
    if (stringify->IsFunction())
    {
      String::Utf8Value str(Handle<Function>::Cast(stringify)->Call(
        Handle<Object>::Cast(json),
        1, 
        &obj
      ));
      return String::New(ToCString(str));
    }
  }
  return ThrowException(String::New("Could not JSONify the object"));
}

