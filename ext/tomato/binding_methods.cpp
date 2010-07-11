#include "tomato.h"
#include "bindings.h"

static VALUE call_rb_bound_method(VALUE args);
static Handle<Value> bound_method(const Arguments& args);

void tomatofy_function(Handle<Function> function, V8Tomato *tomato, VALUE receiver, Handle<Value> rb_method_name)
{
  register_value_wrapper(function, tomato, receiver);
  function->Set(String::New("_tomato_rb_method_name"), rb_method_name);
}

static Handle<Value> call_js_bound_method(const Arguments& args)
{
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
  
  result = rb_protect(call_rb_bound_method, rbargs, &error);
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

static VALUE call_rb_bound_method(VALUE args)
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
  ValueWrapper *wrapper = extract_value_wrapper(function);
  Local<Value> rb_method_name    = function->Get(String::New("_tomato_rb_method_name"));
  if (!rb_method_name->IsString()) throw std::string("Error: _tomato_rb_method_name is not a String (BUG: please report)");
  
  // get the method name from the function name
  String::Utf8Value method_name(rb_method_name);

  // store the output and return success.
  *out_tomato = wrapper->tomato;
  *out_method_id = rb_intern(ToCString(method_name));
  *out_receiver = wrapper->value;
  return 0;
}

void bind_method(V8Tomato *tomato, VALUE reference, Handle<Object> object, const char *rb_method_name, const char *js_method_name)
{
  Handle<Function> function = FunctionTemplate::New(call_js_bound_method)->GetFunction();
  tomatofy_function(function, tomato, reference, String::New(rb_method_name));
  function->SetName(String::New(js_method_name));
  object->Set(String::New(js_method_name), function);
}

VALUE fTomato_bind_method(int argc, VALUE *argv, VALUE self)
{
  if (argc != 4) rb_raise(rb_eArgError,
    "Expected: _bind_method(rb_method_name<str>, receiver<obj>, object_chain<array[string] or nil>, rb_method_name<str>)");
  const char *rb_method_name = rb_id2name(rb_to_id(argv[0]));
  const char *js_method_name = rb_id2name(rb_to_id(argv[3]));
  VALUE reference = argv[1];
  
  V8Tomato *tomato;
  Data_Get_Struct(self, V8Tomato, tomato);
  
  HandleScope handle_scope;
  Context::Scope context_scope(tomato->context);
  Handle<Value> value = find_or_create_object_chain(tomato, argv[2]);
  if (value->IsObject())
  {
    bind_method(tomato, reference, Handle<Object>::Cast(value), rb_method_name, js_method_name);
    return Qtrue;
  }
  return Qfalse;
}
