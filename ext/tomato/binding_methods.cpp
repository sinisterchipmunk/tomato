#include "tomato.h"
#include "binding_methods.h"

static VALUE bound_method_call(VALUE args);

void tomatofy_function(Handle<Function> function, V8Tomato *tomato, int receiver_index, Handle<Value> rb_method_name)
{
  function->Set(String::New("_tomato"), External::New(tomato));
  function->Set(String::New("_tomato_receiver_index"), Int32::New(receiver_index));
  function->Set(String::New("_tomato_rb_method_name"), rb_method_name);
}


Handle<Value> bound_method(const Arguments& args)
{
  // we get the method ID and then call it, so that any C++ destructors that need to fire before
  // we do so, can.
  VALUE receiver;
  ID rb_method_id;
  V8Tomato *tomato;
  VALUE result;

  TRY_JS(store_rb_message(args, &tomato, &receiver, &rb_method_id));

  VALUE rbargs = rb_ary_new2(2+args.Length());
  rb_ary_store(rbargs, 0, receiver);
  rb_ary_store(rbargs, 1, ID2SYM(rb_method_id));
  store_args(tomato, rbargs, args);
  int error;
  
  result = rb_protect(bound_method_call, rbargs, &error);
  if (error) return ThrowException(js_error_from(rb_gv_get("$!")));
  return js_value_of(tomato, result);
}

void store_args(V8Tomato *tomato, VALUE rbargs, const Arguments &args)
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
  Local<Value> rb_method_name    = function->Get(String::New("_tomato_rb_method_name"));
  
  if (!v8_tomato->IsExternal())      throw std::string("Error: _tomato is not an object (BUG: please report)");
  if (!v8_receiver_index->IsInt32()) throw std::string("Error: _tomato_receiver_index is not an Int32 (BUG: please report)");
  if (!rb_method_name->IsString())   throw std::string("Error: _tomato_rb_method_name is not a String (BUG: please report)");
    
  // find the tomato
  V8Tomato *tomato = (V8Tomato *)Local<External>::Cast(v8_tomato)->Value();
  
  // find the receiver index, and make sure it's a valid index
  int receiver_index = v8_receiver_index->Int32Value();
  VALUE receivers = rb_iv_get(tomato->rb_instance, "@receivers"); //rb_funcall(tomato->rb_instance, rb_intern("receivers"));
  if (RARRAY_LEN(receivers) < receiver_index) throw std::string("Error: _tomato_receiver_index is greater than @receivers.length (BUG: please report)");
  
  // get the receiver
  VALUE receiver = (RARRAY_PTR(receivers)[receiver_index]);

  // get the method name from the function name
  String::Utf8Value method_name(rb_method_name);//function->GetName());

  // store the output and return success.
  *out_tomato = tomato;
  *out_method_id = rb_intern(ToCString(method_name));
  *out_receiver = receiver;
  return 0;
}

VALUE fTomato_bind_method(int argc, VALUE *argv, VALUE self)
{
  if (argc != 4) rb_raise(rb_eArgError,
    "Expected: _bind_method(rb_method_name<str>, receiver_index<int>, object_chain<array[string] or nil, rb_method_name<str>>)");
  const char *rb_method_name = rb_id2name(rb_to_id(argv[0]));
  const char *js_method_name = rb_id2name(rb_to_id(argv[3]));
  
  V8Tomato *tomato;
  Data_Get_Struct(self, V8Tomato, tomato);
  
  HandleScope handle_scope;
  Context::Scope context_scope(tomato->context);
  Handle<Value> value = find_or_create_object_chain(tomato, argv[2]);
  if (value->IsObject())
  {
    Handle<Object> object = Handle<Object>::Cast(value);
    Handle<Function> function = FunctionTemplate::New(bound_method)->GetFunction();

    tomatofy_function(function, tomato, FIX2INT(argv[1]), String::New(rb_method_name));
    function->SetName(String::New(js_method_name));

    object->Set(String::New(js_method_name), function);
    return Qtrue;
  }
  return Qfalse;
}
