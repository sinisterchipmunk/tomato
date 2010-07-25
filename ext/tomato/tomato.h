#ifndef TOMATO_H
#define TOMATO_H

/**
  The Tomato::V8 class handles low-level interfacing between Ruby and V8. It does things like translate Ruby objects to JS and vice
  versa.

  The Tomato class wraps around this and provides higher-level interfacing such as binding objects, generating object chains, etc.
  While Tomato still has to manage objects of type Handle<Value> and related, it should not have to interface directly with the V8
  context, as that is Tomato::V8's job.
**/

#include <string>
#include <ruby.h>
#include <v8.h>

using namespace std;
using namespace v8;

#include "config.h"

/* for 1.8.7 compatibility */
#ifndef DBL2NUM
  #define DBL2NUM(n) rb_float_new(n)
#endif

// Extracts a C string from a V8 Utf8Value.
#define ToCString(value) (*value ? *value : "<string conversion failed>")

#define WRAP_CPP(x) try { x; }                                           \
                    catch(const std::string &s) { rb_raise(cTomatoError, s.c_str(), ""); } \
                    catch (const VALUE &exc) { rb_exc_raise(exc); }

/* errors.cpp */
extern VALUE WRAP_RB(VALUE (*meth)(VALUE), VALUE args);
extern void throw_error(TryCatch *try_catch);

/* tomato.cpp */
class Tomato
{
  private:
    VALUE rb_instance;
    VALUE rb_references;

  public:
    Tomato(VALUE klass);
    ~Tomato();
    void rb_gc_mark();
    VALUE instance() { return this->rb_instance; }
};

extern "C" void Init_tomato(void);
extern VALUE cTomato;
extern VALUE cTomatoError;

/* v8.cpp */
class TomatoV8
{
  private:
    VALUE rb_instance;
    Persistent<Context> js_context;

    VALUE compile_and_run(Handle<String> source, Handle<Value> name);

  public:
    TomatoV8(VALUE klass);
    ~TomatoV8();
    Persistent<Context> context() { return js_context; }
    VALUE instance() { return rb_instance; }

    VALUE execute(const char *javascript, const char *filename);
};

extern void Init_v8(void);

extern VALUE cV8;

/* in wrappers.cpp */
class JavascriptValue
{
  private:
    Handle<Value> value;

  public:
    JavascriptValue(Handle<Value> value) { this->value = value; }
    Handle<Value> toJSON(Handle<Value> json);
    VALUE toRuby();
    VALUE toRubyString();
    VALUE toRubyNumber();
    VALUE toRubyBoolean();
    VALUE toRubyDate();
    VALUE toRubyArray();
};

/* debug.cpp */
extern void Init_debug(void);
extern void trace(VALUE string);
extern void trace(const char *string, ...);

#endif//TOMATO_H
