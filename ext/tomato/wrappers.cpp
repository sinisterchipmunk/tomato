#include "tomato.h"

static VALUE cRubyValue = Qnil;

static VALUE fRubyValue_inspect(VALUE self);

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
RubyValue::RubyValue()
{
  this->rb_self = Data_Wrap_Struct(cRubyValue, RB_MARK_FUNC(RubyValue), RB_FREE_FUNC(RubyValue), this);
}

RubyValue::~RubyValue()
{
  trace("Disposing of RubyValue<%x>", this);
  this->js_value.Dispose();
}

static Handle<Value> bound_valueOf(const Arguments &args)
{
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
  return Null();
}

void RubyValue::set(VALUE value)
{
  #if DEBUG == 1
    VALUE inspection = rb_funcall(value, rb_intern("inspect"), 0);
    trace("creating RubyValue<%x> from %s", this, StringValuePtr(inspection));
  #endif
  this->rb_value = value;
  trace("  - creating bound object");
  Handle<Object> bound_object = Object::New();
  
  trace("  - adding valueOf() function");
  Local<Function> func = FunctionTemplate::New(bound_valueOf)->GetFunction();
  func->SetName(String::New("valueOf"));
  bound_object->Set(String::New("valueOf"), func);
  
  trace("  - making bound object persistent");
  this->js_value = Persistent<Object>::New(bound_object);
  trace("  - setting ruby_value on bound object");
  this->js_value->SetHiddenValue(String::New("ruby_value"), External::New(this));
//  trace("  - making bound object weak");
//  this->js_value.MakeWeak(NULL, &UnbindValue);
}

Handle<Value> RubyValue::toJavascriptPrimitive()
{
  if (NIL_P(rb_value))      return Null();
  if (FIXNUM_P(rb_value))   return Number::New(FIX2INT(rb_value));
//  if (value->IsString())    return toRubyString();
//  if (value->IsNumber())    return toRubyNumber();
//  if (value->IsFunction())  return toRubyString();
//  if (value->IsUndefined()) return ID2SYM(rb_intern("undefined"));
//  if (value->IsBoolean())   return toRubyBoolean();
//  if (value->IsDate())      return toRubyDate();
//  if (value->IsArray())     return toRubyArray();
  return String::New(inspect());
}
