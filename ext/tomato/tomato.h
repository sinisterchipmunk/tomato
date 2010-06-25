#ifndef TOMATO_H
#define TOMATO_H

#include <string>
#include <ruby.h>
#include <v8.h>

using namespace v8;

typedef VALUE (ruby_method_vararg)(...);
typedef VALUE (ruby_method_1)(VALUE);

typedef struct {
  Persistent<Context> context;
  VALUE rb_instance;
} V8Tomato;

// Extracts a C string from a V8 Utf8Value.
#define ToCString(value) (*value ? *value : "<string conversion failed>")

/* in tomato.cpp */
extern VALUE cTomato;
extern VALUE cTomatoError;
extern VALUE rb_cTime;

/* in conversions_to_rb.cpp */
extern VALUE ruby_value_of(V8Tomato *tomato, Handle<Value> result);
extern void  inspect_js(V8Tomato *tomato, Handle<Value> obj);

/* in conversions_to_js.cpp */
extern Handle<Value> js_value_of(V8Tomato *tomato,  VALUE value);
extern Handle<Value> inspect_rb(VALUE value);

/* in errors.cpp */
extern void raise_error(TryCatch *try_catch);
extern Local<Object> js_error_from(VALUE ruby_error);
extern void err_init(void);

/* in v8.cpp */
extern VALUE execute(V8Tomato *tomato, Handle<String> source, Handle<Value> name);

/* in binding_methods.cpp */
extern VALUE fTomato_bind_method(int argc, VALUE *argv, VALUE self);

#endif//TOMATO_H
