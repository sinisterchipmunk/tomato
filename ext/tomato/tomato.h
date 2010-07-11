#ifndef TOMATO_H
#define TOMATO_H

#include <string>
#include <ruby.h>
#include <v8.h>

using namespace v8;

/* for 1.8.7 compatibility */
#ifndef DBL2NUM
  #define DBL2NUM(n) rb_float_new(n)
#endif

typedef VALUE (ruby_method_vararg)(...);
typedef VALUE (ruby_method_1)(VALUE);

typedef struct {
  Persistent<Context> context;
  VALUE rb_instance;
  VALUE rb_references;
} V8Tomato;

typedef struct {
  VALUE value;
  V8Tomato *tomato;
} ValueWrapper;

// Extracts a C string from a V8 Utf8Value.
#define ToCString(value) (*value ? *value : "<string conversion failed>")

/* in tomato.cpp */
extern VALUE cTomato;
extern VALUE cTomatoError;
extern VALUE rb_cTime;
extern void push_rb_reference(V8Tomato *tomato, VALUE ref);
extern void pop_rb_reference(V8Tomato *tomato, VALUE ref);


/* in object_chain.cpp */
extern Handle<Value> find_or_create_object_chain(V8Tomato *tomato, VALUE chain);
extern VALUE fTomato_bind_class(VALUE self, VALUE klass, VALUE chain);

/* in conversions_to_rb.cpp */
extern VALUE ruby_value_of(V8Tomato *tomato, Handle<Value> result);
extern Handle<Value> inspect_js(V8Tomato *tomato, Handle<Value> obj);

/* in conversions_to_js.cpp */
extern Handle<Value> js_value_of(V8Tomato *tomato,  VALUE value);
extern Handle<Value> inspect_rb(VALUE value);

/* in errors.cpp */
extern void raise_error(TryCatch *try_catch);
extern Local<Object> js_error_from(VALUE ruby_error);
extern void err_init(void);
extern Local<Object> js_error_new(const char *str);

/* in v8.cpp */
extern VALUE execute(V8Tomato *tomato, Handle<String> source, Handle<Value> name);

/* in binding_methods.cpp */
extern VALUE fTomato_bind_method(int argc, VALUE *argv, VALUE self);

/* in value_wrapper.cpp */
extern void register_value_wrapper(Handle<Object> target, V8Tomato *tomato, VALUE value);
extern ValueWrapper *extract_value_wrapper(Handle<Object> target);

#endif//TOMATO_H
