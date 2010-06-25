#include "tomato.h"

static VALUE bound_method_call(VALUE args);
static Handle<Value> bound_method(const Arguments& args);
static int store_rb_message(const Arguments &args, V8Tomato **out_tomato, VALUE *out_receiver, ID *out_method_id);


Handle<Value> bound_method(const Arguments& args)
{
  // we get the method ID and then call it, so that any C++ destructors that need to fire before
  // we do so, can.
  VALUE receiver;
  ID rb_method_id;
  V8Tomato *tomato;
  int code = store_rb_message(args, &tomato, &receiver, &rb_method_id);
  if (code == -1)
    return ThrowException(String::New("Error: _tomato is not an object (BUG: please report)"));

  VALUE rbargs = rb_ary_new2(2);
  rb_ary_store(rbargs, 0, receiver);
  rb_ary_store(rbargs, 1, ID2SYM(rb_method_id));
  
  int error;
  VALUE result = rb_protect(bound_method_call, rbargs, &error);
  if(error) 
  {
    return ThrowException(js_error_from(rb_gv_get("$!")));
  }

  return js_value_of(tomato, result);
}

static VALUE bound_method_call(VALUE args)
{
  VALUE receiver = rb_ary_entry(args, 0);
  VALUE rb_method_id = SYM2ID(rb_ary_entry(args, 1));
  
  VALUE result = rb_funcall(receiver, rb_method_id, 0);
  
  return result;
}

int store_rb_message(const Arguments &args, V8Tomato **out_tomato, VALUE *out_receiver, ID *out_method_id)
{
  Handle<Function> function = args.Callee();
  Handle<String> _tomato = String::New("_tomato");
  if (!function->Get(_tomato)->IsExternal())
    return -1;
    
  V8Tomato *tomato = (V8Tomato *)Local<External>::Cast(function->Get(_tomato))->Value();
  VALUE receiver = tomato->rb_instance;
  String::Utf8Value method_name(function->GetName());

  *out_tomato = tomato;
  *out_method_id = rb_intern(ToCString(method_name));
  *out_receiver = receiver;
  return 0;
}

VALUE fTomato_bind_method(int argc, VALUE *argv, VALUE self)
{
  if (argc != 1) rb_raise(rb_eArgError, "Expected name of method");
  const char *method_name = rb_id2name(rb_to_id(argv[0]));
  
  V8Tomato *tomato;
  Data_Get_Struct(self, V8Tomato, tomato);
  
  HandleScope handle_scope;
  Context::Scope context_scope(tomato->context);  
  Handle<Value> value = tomato->context->Global();
  if (value->IsObject())
  {
    Handle<Object> object = Handle<Object>::Cast(value);
    Handle<Value> proto_value = object->GetPrototype();
    Handle<Function> function = FunctionTemplate::New(bound_method)->GetFunction();

    function->Set(String::New("_tomato"), External::New(tomato));
    function->SetName(String::New(method_name));

    object->Set(String::New(method_name), function);
    return Qtrue;
  }
  return Qfalse;
}
