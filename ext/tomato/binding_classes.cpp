#include "tomato.h"
#include "bindings.h"

static VALUE create_new(VALUE args);
static v8::Handle<v8::Value> ruby_class_constructor(const Arguments &args);
static Handle<Value> bind_methods(Local<Object> js, VALUE rb, V8Tomato *tomato);
static Handle<Value> bound_getter(Local<String> property, const AccessorInfo &info);
static void bound_setter(Local<String> property, Local<Value> value, const AccessorInfo &info);
static VALUE protected_get(VALUE args);
static VALUE protected_set(VALUE args);

VALUE fTomato_bind_class(VALUE self, VALUE receiver, VALUE chain)
{
  V8Tomato *tomato;
  Data_Get_Struct(self, V8Tomato, tomato);
  
  HandleScope handle_scope;
  Context::Scope context_scope(tomato_v8_context(tomato));
  VALUE js_class_name = rb_ary_pop(chain);
  // This is kind of a misnomer. We're creating a JavaScript function ("method") to stand in for
  // the Ruby class. So the method_name has to be the Ruby class name. Consider: "new" is not a 
  // method in JS -- it's a keyword.
  Handle<String> method_name = String::New(StringValuePtr(js_class_name));
  Handle<Value> parent = find_or_create_object_chain(tomato, chain);

  if (parent->IsObject())
  {
    Handle<Object> object = Handle<Object>::Cast(parent);
    Handle<Function> function = FunctionTemplate::New(ruby_class_constructor)->GetFunction();
    
    tomatofy_function(function, tomato, receiver, method_name);
    function->SetName(method_name);

    Handle<Value> current_value = object->Get(method_name);
    if (current_value->IsObject())
    {
      // we're about to overwrite an object. First clone any of its registered functions.
      Handle<Object> current = Handle<Object>::Cast(current_value);
      Local<Array> properties = current->GetPropertyNames();
      int length = properties->Length();
      for (int i = 0; i < length; i++)
      {
        Local<Value> property = properties->Get(i);
        String::Utf8Value stra(inspect_js(tomato, property));
        String::Utf8Value str(inspect_js(tomato, current->Get(property)));
        function->Set(property, current->Get(property));
      }
    }
    object->Set(method_name, function);
    return Qtrue;
  }
  return Qfalse;
}

v8::Handle<v8::Value> ruby_class_constructor(const Arguments &args)
{
  // throw if called without `new'
  if (!args.IsConstructCall()) 
    return ThrowException(String::New("Cannot call constructor as function"));
 
  VALUE receiver;
  ID rb_method_id;
  V8Tomato *tomato;
  TRY_JS(store_rb_message(args, &tomato, &receiver, &rb_method_id));

  VALUE rbargs = rb_ary_new2(1+args.Length());
  rb_ary_store(rbargs, 0, receiver);
  store_args(tomato, rbargs, args);
  
  int error;
  VALUE result = rb_protect(create_new, rbargs, &error);
  if(error) 
  {
    return ThrowException(js_error_from(rb_gv_get("$!")));
  }

  Local<Object> holder = args.Holder();
  register_value_wrapper(holder, tomato, result);  
  return bind_methods(holder, result, tomato);
}

static Handle<Value> bind_methods(Local<Object> js_object, VALUE rb_reference, V8Tomato *tomato)
{
  VALUE methods = rb_funcall(rb_reference, rb_intern("public_methods"), 0);
  
  HandleScope handle_scope;
  Context::Scope context_scope(tomato_v8_context(tomato));
  for (int i = 0; i < RARRAY_LEN(methods); i++)
  {
    const char *method_name = StringValuePtr(*(RARRAY_PTR(methods)+i));
    bind_method(tomato, rb_reference, js_object, method_name, method_name);
    js_object->SetAccessor(String::New(method_name), bound_getter, bound_setter);
  }
  return js_object;
}

static VALUE protected_get(VALUE args)
{
  VALUE receiver = *(RARRAY_PTR(args));
  VALUE method = *(RARRAY_PTR(args)+1);
  return rb_funcall2(receiver, SYM2ID(method), 0, 0);
}

static VALUE protected_set(VALUE args)
{
  VALUE receiver = *(RARRAY_PTR(args));
  VALUE method = *(RARRAY_PTR(args)+1);
  VALUE value = *(RARRAY_PTR(args)+2);
  
  method = rb_funcall(method, rb_intern("to_s"), 0);
  method = rb_funcall(method, rb_intern("+"), 1, rb_str_new2("="));
  return rb_funcall2(receiver, rb_intern(StringValuePtr(method)), 1, &value);
  return Qnil;
}

static Handle<Value> bound_getter(Local<String> property, const AccessorInfo &info)
{
  int error;
  Local<Object> self = info.Holder();
  
  ValueWrapper *wrapper = extract_value_wrapper(self);
  
  VALUE args = rb_ary_new();
  String::Utf8Value property_name(property);
  rb_ary_push(args, wrapper->value);
  rb_ary_push(args, ID2SYM(rb_intern(ToCString(property_name))));
  VALUE result = rb_protect(protected_get, args, &error);
  if (error)
    return ThrowException(js_error_from(rb_gv_get("$!")));
  return js_value_of(wrapper->tomato, result);
}

static void bound_setter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
  int error;
  Local<Object> self = info.Holder();
  ValueWrapper *wrapper = extract_value_wrapper(self);

  VALUE args = rb_ary_new();
  String::Utf8Value property_name(property);
  rb_ary_push(args, wrapper->value);
  rb_ary_push(args, ID2SYM(rb_intern(ToCString(property_name))));
  rb_ary_push(args, ruby_value_of(wrapper->tomato, value));
  rb_protect(protected_set, args, &error);
  if (error)
    ThrowException(js_error_from(rb_gv_get("$!")));
}

static VALUE create_new(VALUE args)
{
  VALUE receiver = rb_ary_shift(args);
  VALUE result = rb_funcall2(receiver, rb_intern("new"), RARRAY_LEN(args), RARRAY_PTR(args));
  return result;
}