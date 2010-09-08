#include "tomato.h"

static VALUE cRubyValue = Qnil;

static VALUE fRubyValue_inspect(VALUE self);
static Handle<Value> bound_valueOf(const Arguments &args);

static RubyValue *RB_ALLOC_FUNC(RubyValue)(VALUE klass)
{ /* klass is the same as class_name, so no need to use it directly. */
  RubyValue *obj = new RubyValue();
  return obj;
}

RB_DEFINE_MARK_FUNC(RubyValue);
RB_DEFINE_FREE_FUNC(RubyValue);

void Init_wrappers()
{
  cRubyValue = rb_define_class_under(cTomato, "RubyValue", rb_cObject);
  rb_define_alloc_func(cRubyValue, (VALUE(*)(VALUE))&fRubyValue_allocate);
  rb_define_method(cRubyValue, "inspect", (VALUE(*)(...))&fRubyValue_inspect, 0);
  
//  rb_define_method(cTomato, "run", (VALUE(*)(...))&fTomato_run, -1);
//  rb_define_method(cTomato, "bind_object", (VALUE(*)(...))&fTomato_bind_object, -1);
}

static VALUE fRubyValue_inspect(VALUE self)
{
  //trace(rb_str_new4(self));
  RB_EXTRACT_STRUCT(self, RubyValue, wrapper);
  VALUE inspection = rb_str_new2("RubyValue:");
  rb_str_cat2(inspection, wrapper->inspect());
  return inspection;
}

/* class JavascriptValue */
Handle<Value> JavascriptValue::toJSON(Handle<Value> json)
{
  /* Call Javascript's JSON.stringify(object) method. If that can't be done for any reason, return an error. */
  if (json->IsObject())
  {
    Handle<Value> stringify = Handle<Object>::Cast(json)->Get(String::New("stringify"));
    if (stringify->IsFunction())
    {
      String::Utf8Value str(Handle<Function>::Cast(stringify)->Call(
        Handle<Object>::Cast(json),
        1,
        &(this->value)
      ));
      return String::New(ToCString(str));
    }
  }
  return ThrowException(String::New("Could not JSONify the object"));
}

VALUE JavascriptValue::toRuby()
{
  if (value->IsNull())      return Qnil;
  if (value->IsString())    return toRubyString();
  if (value->IsNumber())    return toRubyNumber();
  if (value->IsFunction())  return toRubyString();
  if (value->IsUndefined()) return ID2SYM(rb_intern("undefined"));
  if (value->IsBoolean())   return toRubyBoolean();
  if (value->IsDate())      return toRubyDate();
  if (value->IsArray())     return toRubyArray();
  
  throw string("json");
}

VALUE JavascriptValue::toRubyString()
{
  String::Utf8Value str(value->ToString());
  return rb_str_new2(ToCString(str));
}

VALUE JavascriptValue::toRubyNumber()
{
  if (value->IsInt32())  return INT2FIX(value->Int32Value());
  if (value->IsUint32()) return INT2FIX(value->Uint32Value());
  return DBL2NUM(value->NumberValue());
}

VALUE JavascriptValue::toRubyBoolean()
{
  if (value->IsTrue()) return Qtrue;
  return Qfalse;
}

VALUE JavascriptValue::toRubyDate()
{
  const Handle<Date> date = Handle<Date>::Cast(value);
  return rb_funcall(rb_cTime, rb_intern("at"), 1, DBL2NUM(date->NumberValue() / 1000.0));
}

VALUE JavascriptValue::toRubyArray()
{
  Handle<Array> array = Handle<Array>::Cast(value);
  unsigned int length = array->Length();
  VALUE rbarr = rb_ary_new2(length);
  for (unsigned int i = 0; i < length; i++)
  {
    JavascriptValue val(array->Get(Integer::New(i)));
    rb_ary_push(rbarr, val.toRuby());
  }
  return rbarr;
}


/* class RubyValue */
void RubyValue::init()
{
  this->rb_self = Data_Wrap_Struct(cRubyValue, RB_MARK_FUNC(RubyValue), RB_FREE_FUNC(RubyValue), this);


  trace("creating RubyValue object template");
  Local<FunctionTemplate> functemp = FunctionTemplate::New();
  
  
  //this->object_template = functemp->PrototypeTemplate();
  
  INC_TRACE_DEPTH;
    trace("- adding named property interceptor");
    //this->object_template->SetNamedPropertyHandler(named_get, named_set);
    
    trace("- adding indexed property interceptor");
    //this->object_template->SetIndexedPropertyHandler(this->indexed_get, this->indexed_set);
    
    trace("- adding valueOf() function");
    this->object_template->Set(String::New("valueOf"), FunctionTemplate::New(bound_valueOf));

    //trace("- adding proxy functions");
    //this->js_value->
  DEC_TRACE_DEPTH;
}

RubyValue::~RubyValue()
{
  trace("Disposing of RubyValue<%x>", this);
  this->js_value.Dispose();
}

static Handle<Value> bound_valueOf(const Arguments &args)
{
  printf("1\n");
  Local<Object> self = args.Holder();
  trace("bound_object#valueOf()");
  Handle<Value> value = self->GetHiddenValue(String::New("ruby_value"));
  if (value->IsExternal())
  {
    RubyValue *ruby = (RubyValue *)External::Unwrap(value);
    if (ruby)
      return ruby->toJavascriptPrimitive();
    else
      return ThrowException(String::New("(Tomato) BUG: Bound value could not be unwrapped!"));
  }
  else
    return ThrowException(String::New("(Tomato) BUG: Bound value is not an External!"));
}

void RubyValue::set(VALUE value)
{
  #if DEBUG == 1
    VALUE inspection = rb_funcall(value, rb_intern("inspect"), 0);
    trace("creating RubyValue<%x> from %s", this, StringValuePtr(inspection));
  #endif
  this->rb_value = value;
  
  INC_TRACE_DEPTH;
    trace("creating bound object");
    Handle<Object> bound_object = this->object_template->NewInstance();
    
    trace("making bound object persistent");
    this->js_value = Persistent<Object>::New(bound_object);
    
    trace("setting ruby_value on bound object");
    this->js_value->SetHiddenValue(String::New("ruby_value"), External::New(this));
  DEC_TRACE_DEPTH;
}

Handle<Value> RubyValue::named_get(Local<String> name, const AccessorInfo &info)
{
  return Null();
}

Handle<Value> RubyValue::named_set(Local<String> name, Handle<Value> new_value, const AccessorInfo &info)
{
  return Null();
}

Handle<Value> RubyValue::indexed_get(uint32_t index, const AccessorInfo &info)
{
  return Null();
}

Handle<Value> RubyValue::indexed_set(uint32_t index, Local<Value> new_value, const AccessorInfo &info)
{
  return Null();
}


Handle<Value> RubyValue::toJavascript()
{
  if (NIL_P(rb_value)) return Null();
  
//  switch(TYPE(rb_value))
//  {
//    case T_ARRAY:  return toJavascriptArray();
//  };
  return this->js_value;
}

Handle<Array> RubyValue::toJavascriptPrimitiveArray()
{
  trace("RubyValue<%x>::toJavascriptPrimitiveArray()", this);
  int i, len = RARRAY_LEN(rb_value);
  VALUE element;
  INC_TRACE_DEPTH;
    trace("- initializing array with %d elements", len);
    Handle<Array> array = Array::New(len);
  
    INC_TRACE_DEPTH;
      for (i = 0; i < RARRAY_LEN(rb_value); i++)
      {
        element = *(RARRAY_PTR(rb_value)+i);
        
        #if DEBUG == 1   
          VALUE inspection = rb_funcall(element, rb_intern("inspect"), 0);
          trace("setting element %d to %s", i, StringValuePtr(inspection));
        #endif
        
        INC_TRACE_DEPTH;
          array->Set(i, Number::New(i));
          //RubyValue value(element);
          //array->Set(i, value.toJavascriptPrimitive());
        DEC_TRACE_DEPTH;
      }
    DEC_TRACE_DEPTH;

  #if DEBUG == 1
    trace("array is built, reverse check to follow:");
    INC_TRACE_DEPTH;
      JavascriptValue js(array);
      trace("inspecting value...");
      VALUE v = rb_funcall(js.toRuby(), rb_intern("inspect"), 0);
      trace("result: %s", StringValuePtr(v));
    DEC_TRACE_DEPTH;
  #endif

  DEC_TRACE_DEPTH;
  return array;
}
    
Handle<Value> RubyValue::toJavascriptPrimitive()
{
  /* can't return +null+ here; see #toJavascript() instead */

  #if DEBUG == 1
    VALUE inspection = rb_funcall(rb_value, rb_intern("inspect"), 0);
    trace("RubyValue<%x>#toJavascriptPrimitive() <%s>", this, StringValuePtr(inspection));
  #endif
  
  if (FIXNUM_P(rb_value))
    return Number::New(FIX2INT(rb_value));
    
  switch(TYPE(rb_value))
  {
    case T_FLOAT:  return Number::New(NUM2DBL(rb_value));
    case T_STRING: return String::New(StringValuePtr(rb_value));
    case T_REGEXP: return ThrowException(String::New("(Tomato) Not Implemented: conversion from T_REGEXP to v8::RegExp is not possible at this time"));
    case T_ARRAY:  return toJavascriptPrimitiveArray();
  };
  /*
	T_OBJECT	ordinary object
	T_CLASS		class
	T_MODULE	module
	T_FLOAT		floating point number
	T_STRING	string
	T_REGEXP	regular expression
	T_ARRAY		array
	T_HASH		associative array
	T_STRUCT	(Ruby) structure
	T_BIGNUM	multi precision integer
	T_FIXNUM	Fixnum(31bit or 63bit integer)
	T_COMPLEX       complex number
	T_RATIONAL      rational number
	T_FILE		IO
	T_TRUE		true
	T_FALSE		false
	T_DATA		data
	T_SYMBOL        symbol
  */
//  if (value->IsString())    return toRubyString();
//  if (value->IsNumber())    return toRubyNumber();
//  if (value->IsFunction())  return toRubyString();
//  if (value->IsUndefined()) return ID2SYM(rb_intern("undefined"));
//  if (value->IsBoolean())   return toRubyBoolean();
//  if (value->IsDate())      return toRubyDate();
//  if (value->IsArray())     return toRubyArray();
  return String::New(inspect());
}
