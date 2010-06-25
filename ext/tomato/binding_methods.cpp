#include "tomato.h"

static VALUE bound_method_call(VALUE args);
static Handle<Value> bound_method(const Arguments& args);
static int store_rb_message(const Arguments &args, V8Tomato **out_tomato, VALUE *out_receiver, ID *out_method_id);
static void store_args(V8Tomato *tomato, VALUE rbargs, const Arguments &args);

Handle<Value> bound_method(const Arguments& args)
{
  // we get the method ID and then call it, so that any C++ destructors that need to fire before
  // we do so, can.
  VALUE receiver;
  ID rb_method_id;
  V8Tomato *tomato;
  int code = store_rb_message(args, &tomato, &receiver, &rb_method_id);
  switch(code)
  {
    case -1: return ThrowException(String::New("Error: _tomato is not an object (BUG: please report)"));
    case -2: return ThrowException(String::New("Error: _tomato_receiver_index is not an Int32 (BUG: please report)"));
    case -3: return ThrowException(String::New("Error: _tomato_receiver_index is greater than @receivers.length (BUG: please report)"));
  };

  VALUE rbargs = rb_ary_new2(2+args.Length());
  rb_ary_store(rbargs, 0, receiver);
  rb_ary_store(rbargs, 1, ID2SYM(rb_method_id));
  store_args(tomato, rbargs, args);
  
  int error;
  VALUE result = rb_protect(bound_method_call, rbargs, &error);
  if(error) 
  {
    return ThrowException(js_error_from(rb_gv_get("$!")));
  }

  return js_value_of(tomato, result);
}

static void store_args(V8Tomato *tomato, VALUE rbargs, const Arguments &args)
{
  int length = args.Length();
  int offset = RARRAY_LEN(rbargs);
  for (int i = 0; i < length; i++)
    rb_ary_store(rbargs, offset+i, ruby_value_of(tomato, args[i]));
}

static VALUE bound_method_call(VALUE args)
{
  VALUE receiver = rb_ary_shift(args);
  VALUE rb_method_id = SYM2ID(rb_ary_shift(args));
  VALUE result = rb_funcall2(receiver, rb_method_id, RARRAY_LEN(args), RARRAY_PTR(args));
  return result;
}

int store_rb_message(const Arguments &args, V8Tomato **out_tomato, VALUE *out_receiver, ID *out_method_id)
{
  // get the function
  Handle<Function> function = args.Callee();
  
  // pull the binding data from the function (stored there by fTomato_bind_method)
  Local<Value> v8_tomato         = function->Get(String::New("_tomato"));
  Local<Value> v8_receiver_index = function->Get(String::New("_tomato_receiver_index"));
  
  // make sure the data is what we expect it to be
  if (!v8_tomato->IsExternal())      return -1;
  if (!v8_receiver_index->IsInt32()) return -2;
    
  // find the tomato
  V8Tomato *tomato = (V8Tomato *)Local<External>::Cast(v8_tomato)->Value();
  
  // find the receiver index, and make sure it's a valid index
  int receiver_index = v8_receiver_index->Int32Value();
  VALUE receivers = rb_iv_get(tomato->rb_instance, "@receivers"); //rb_funcall(tomato->rb_instance, rb_intern("receivers"));
  if (RARRAY_LEN(receivers) < receiver_index) return -3;
  
  // get the receiver
  VALUE receiver = (RARRAY_PTR(receivers)[receiver_index]);

  // get the method name from the function name
  String::Utf8Value method_name(function->GetName());

  // store the output and return success.
  *out_tomato = tomato;
  *out_method_id = rb_intern(ToCString(method_name));
  *out_receiver = receiver;
  return 0;
}

VALUE fTomato_bind_method(int argc, VALUE *argv, VALUE self)
{
  if (argc != 2) rb_raise(rb_eArgError, "Expected: _bind_method(method_name<str>, receiver_index<int>)");
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
    function->Set(String::New("_tomato_receiver_index"), Int32::New(FIX2INT(argv[1])));
    function->SetName(String::New(method_name));

    object->Set(String::New(method_name), function);
    return Qtrue;
  }
  return Qfalse;
}
