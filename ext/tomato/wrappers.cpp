#include "tomato.h"

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
