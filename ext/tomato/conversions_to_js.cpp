#include "tomato.h"

static Handle<Value> js_array_from(V8Tomato *tomato, VALUE value);
static Handle<Value> js_hash_from(V8Tomato *tomato, VALUE value);
static Handle<Value> js_symbol_to_string(const Arguments& args);
static Handle<Value> js_symbol_from(VALUE value);

Handle<Value> js_value_of(V8Tomato *tomato,  VALUE value)
{
  switch(TYPE(value))
  {
    case T_NIL     : return Null();
    //case T_OBJECT  :
    //case T_CLASS   :
    //case T_MODULE  :
    case T_FLOAT   : return Number::New(NUM2DBL(value));
    case T_STRING  : return String::New(StringValuePtr(value));
//    case T_REGEXP  :
    case T_ARRAY   : return js_array_from(tomato, value);
    case T_HASH    : return js_hash_from(tomato, value);
    //case T_STRUCT  :
    case T_BIGNUM  : return Number::New(NUM2LONG(value));
    case T_FIXNUM  : return Int32::New(FIX2INT(value));
    //case T_COMPLEX :
    //case T_RATIONAL:
    //case T_FILE    :
    case T_TRUE    : return True();
    case T_FALSE   : return False();
    //case T_DATA    :
    case T_SYMBOL  :
      if (SYM2ID(value) == rb_intern("undefined")) return Undefined();
      else return js_symbol_from(value);
      //return String::New(rb_id2name(SYM2ID(value)));
  };
  return inspect_rb(value);  
}

Handle<Value> js_array_from(V8Tomato *tomato, VALUE value)
{
  Handle<Array> array;
  int size, i;
  VALUE *ptr;
  
  switch(TYPE(value))
  {
    case T_ARRAY:
      size = RARRAY_LEN(value);
      ptr  = RARRAY_PTR(value);
      array = Array::New(size);
      for (i = 0; i < size; i++)
        array->Set(i, js_value_of(tomato, *(ptr+i)));
      break;
    default:
      return ThrowException(String::New("Could not construct JS array: unexpected Ruby type (BUG: please report)"));
  }
  return array;
}

static Handle<Value> js_symbol_from(VALUE value)
{
  ID id = SYM2ID(value);
  Handle<Object> symbol = Object::New();
  symbol->Set(String::New("_tomato_symbol"), Boolean::New(true), DontEnum);
  symbol->Set(String::New("symbol"), String::New(rb_id2name(id)));

  // make it transparently convert to string
  Handle<Function> function = FunctionTemplate::New(js_symbol_to_string)->GetFunction();
  function->SetName(String::New("toString"));
  function->Set(String::New("symbol"), symbol, DontEnum);
  symbol->Set(String::New("toString"), function);
  return symbol;
}

static Handle<Value> js_symbol_to_string(const Arguments& args)
{
  Handle<Function> function = args.Callee();
  Handle<Object> symbol = Handle<Object>::Cast(function->Get(String::New("symbol")));
  if (!symbol->IsObject())
    return ThrowException(String::New("Could not convert symbol to string: symbol handle missing (BUG: please report)"));
  return symbol->Get(String::New("symbol"));
}

Handle<Value> js_hash_from(V8Tomato *tomato, VALUE value)
{
  VALUE rb_keys   = rb_funcall(value, rb_intern("keys"), 0);
  VALUE rb_values = rb_funcall(value, rb_intern("values"), 0);
  VALUE *keys = RARRAY_PTR(rb_keys),
        *values = RARRAY_PTR(rb_values);

  int size = RARRAY_LEN(rb_keys), i;

  Handle<Object> js_hash = Object::New();
  Handle<Array> js_keys = Array::New(size);
  Handle<Array> js_values = Array::New(size);
  
  for (i = 0; i < size; i++)
  {
    js_keys->Set(i,   js_value_of(tomato, *(keys+i)));
    js_values->Set(i, js_value_of(tomato, *(values+i)));
  }
  
  js_hash->Set(String::New("_tomato_hash_keys"), js_keys, DontEnum);
  js_hash->Set(String::New("_tomato_hash_values"), js_values, DontEnum);
  js_hash->Set(String::New("_tomato_hash"), Boolean::New(true), DontEnum);
  
  return js_hash;
}

Handle<Value> inspect_rb(VALUE value)
{
  VALUE string = rb_funcall(value, rb_intern("inspect"), 0);
  return String::New(StringValuePtr(string));
}

/*
Handle<Value> v8_value_of(V8Tomato *tomato, VALUE object)
{

  switch(TYPE(object))
  {
    case T_NIL     :
    case T_OBJECT  :
    //case T_CLASS   :
    //case T_MODULE  :
    case T_FLOAT   :
    case T_STRING  :
    case T_REGEXP  :
    case T_ARRAY   :
    case T_HASH    :
    //case T_STRUCT  :
    case T_BIGNUM  :
    case T_FIXNUM  :
    //case T_COMPLEX :
    //case T_RATIONAL:
    //case T_FILE    :
    case T_TRUE    :
    case T_FALSE   :
    //case T_DATA    :
    case T_SYMBOL  :
      symbol_from(object)
      break;
    default:         string_from(object);
  };
}
*/
/*
  if (result->IsUndefined()) { return ID2SYM(rb_intern("undefined")); }
  if (result->IsNull())      { return Qnil; }
  if (result->IsTrue())      { return Qtrue; }
  if (result->IsFalse())     { return Qfalse; }
  if (result->IsString())    { return ruby_string_from(result); }
  if (result->IsFunction())  { return ruby_string_from(result); }
  if (result->IsArray())     { Handle<Array> array = Handle<Array>::Cast(result); return ruby_array_from(tomato, array); }
  if (result->IsNumber())    { return ruby_numeric_from(result); }
  if (result->IsDate())      { return ruby_date_from(result); }
    

  if (result->IsObject())
  {
    Handle<Value> json = tomato->context->Global()->Get(String::New("JSON"));
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
        return rb_str_new2(ToCString(str));
      }
    }
  }
  return Qnil;
}
*/
