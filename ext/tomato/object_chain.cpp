#include "tomato.h"
#include "binding_methods.h"

static VALUE create_new(VALUE args);
static v8::Handle<v8::Value> ruby_class_constructor(const Arguments &args);
static Handle<Value> bind_methods(Local<Object> js, VALUE rb, V8Tomato *tomato);
static Handle<Value> bound_getter(Local<String> property, const AccessorInfo &info);
static void bound_setter(Local<String> property, Local<Value> value, const AccessorInfo &info);
static VALUE protected_get(VALUE args);
static VALUE protected_set(VALUE args);

VALUE fTomato_bind_class(VALUE self, VALUE receiver_index, VALUE chain)
{
  V8Tomato *tomato;
  Data_Get_Struct(self, V8Tomato, tomato);
  
  HandleScope handle_scope;
  Context::Scope context_scope(tomato->context);
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
    
    tomatofy_function(function, tomato, FIX2INT(receiver_index), method_name);
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
 
  // we get the method ID and then call it, so that any C++ destructors that need to fire before
  // we do so, can.
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
  holder->Set(String::New("_tomato_ruby_wrapper"), Boolean::New(true), DontEnum);
  holder->Set(String::New("_tomato_receiver_index"),
    Int32::New(FIX2INT(rb_funcall(tomato->rb_instance, rb_intern("receiver_index"), 1, result))), DontEnum);
  return bind_methods(holder, result, tomato);
}

Handle<Value> bind_methods(Local<Object> js, VALUE rb, V8Tomato *tomato)
{
  VALUE methods = rb_funcall(rb, rb_intern("public_methods"), 0);
  int receiver_index = FIX2INT(rb_funcall(tomato->rb_instance, rb_intern("receiver_index"), 1, rb));
  
  HandleScope handle_scope;
  Context::Scope context_scope(tomato->context);
  Handle<String> method_name;
  js->Set(String::New("_tomato"), External::New(tomato));
  for (int i = 0; i < RARRAY_LEN(methods); i++)
  {
    method_name = String::New(StringValuePtr(*(RARRAY_PTR(methods)+i)));
    Handle<Function> function = FunctionTemplate::New(bound_method)->GetFunction();
  
    tomatofy_function(function, tomato, receiver_index, method_name);
    function->SetName(method_name);
  
    js->Set(method_name, function);
    js->SetAccessor(method_name, bound_getter, bound_setter);
  }
  return js;
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

  // pull the binding data from the function (stored there by fTomato_bind_method)
  Local<Value> v8_tomato         = self->Get(String::New("_tomato"));
  Local<Value> v8_receiver_index = self->Get(String::New("_tomato_receiver_index"));
  
  // make sure the data is what we expect it to be
  if (!v8_tomato->IsExternal())      return ThrowException(String::New("_tomato is not an external! (bug: please report)"));
  if (!v8_receiver_index->IsInt32()) return ThrowException(String::New("_tomato_receiver_index is not an Int32! (bug: please report)"));
    
  // find the tomato
  V8Tomato *tomato = (V8Tomato *)Local<External>::Cast(v8_tomato)->Value();
  
  // find the receiver index, and make sure it's a valid index
  int receiver_index = v8_receiver_index->Int32Value();
  VALUE receivers = rb_iv_get(tomato->rb_instance, "@receivers"); //rb_funcall(tomato->rb_instance, rb_intern("receivers"));
  if (RARRAY_LEN(receivers) < receiver_index) return ThrowException(String::New("_tomato_receiver_index is too small! (bug: please report)"));
  
  // get the receiver
  VALUE receiver = (RARRAY_PTR(receivers)[receiver_index]);

  VALUE args = rb_ary_new();
  String::Utf8Value property_name(property);
  rb_ary_push(args, receiver);
  rb_ary_push(args, ID2SYM(rb_intern(ToCString(property_name))));
  VALUE result = rb_protect(protected_get, args, &error);
  if (error)
    return ThrowException(js_error_from(rb_gv_get("$!")));
  return js_value_of(tomato, result);
}

static void bound_setter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
  int error;
  Local<Object> self = info.Holder();

  // pull the binding data from the function (stored there by fTomato_bind_method)
  Local<Value> v8_tomato         = self->Get(String::New("_tomato"));
  Local<Value> v8_receiver_index = self->Get(String::New("_tomato_receiver_index"));
  
  // make sure the data is what we expect it to be
  if (!v8_tomato->IsExternal())      { ThrowException(String::New("_tomato is not an external! (bug: please report)")); return; }
  if (!v8_receiver_index->IsInt32()) { ThrowException(String::New("_tomato_receiver_index is not an Int32! (bug: please report)")); return; }
    
  // find the tomato
  V8Tomato *tomato = (V8Tomato *)Local<External>::Cast(v8_tomato)->Value();
  
  // find the receiver index, and make sure it's a valid index
  int receiver_index = v8_receiver_index->Int32Value();
  VALUE receivers = rb_iv_get(tomato->rb_instance, "@receivers"); //rb_funcall(tomato->rb_instance, rb_intern("receivers"));
  if (RARRAY_LEN(receivers) < receiver_index) { ThrowException(String::New("_tomato_receiver_index is too small! (bug: please report)")); return; }
  
  // get the receiver
  VALUE receiver = (RARRAY_PTR(receivers)[receiver_index]);

  VALUE args = rb_ary_new();
  String::Utf8Value property_name(property);
  rb_ary_push(args, receiver);
  rb_ary_push(args, ID2SYM(rb_intern(ToCString(property_name))));
  rb_ary_push(args, ruby_value_of(tomato, value));
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

Handle<Value> find_or_create_object_chain(V8Tomato *tomato, VALUE chain)
{
  Handle<Value> value = tomato->context->Global();
  if (value->IsObject())
  {
    Handle<Object> object = Handle<Object>::Cast(value);
    if (chain != Qnil)
    {
      int len = RARRAY_LEN(chain);
      VALUE *items = RARRAY_PTR(chain);
      
      for (int i = 0; i < len; i++)
      {
        Handle<String> object_name = String::New(StringValuePtr(items[i]));
        value = object->Get(object_name);
        if (value->IsObject())
        {
          object = Handle<Object>::Cast(value);  
        }
        else
        {
          Handle<Object> new_object = Object::New();
          object->Set(object_name, new_object);
          object = new_object;
        }
      }
    }
    return object;
  }
  return value;
}
