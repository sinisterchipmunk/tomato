#ifndef TOMATO_H
#define TOMATO_H

/**
  The Tomato::Context class handles low-level interfacing between Ruby and V8. It does things like translate Ruby objects to JS and vice
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

#include "dry.h"
#include "errors.h"
#include "tomato_context.h"
#include "debug.h"
#include "wrappers.h"

/* tomato.cpp */
class Tomato
{
  private:
    VALUE rb_instance;
    
    // {object_id => reference(obj)}, e.g. {object_id => [1, obj]} 
    VALUE rb_references;
    
    /* Returns the array used as a value in rb_references, or a new array if none exists.
       The array contains [reference_count, obj] where reference_count is the number of open
       references to obj. */
    VALUE reference(VALUE obj);
  
  protected:
    void add_reference(VALUE obj);
    void remove_reference(VALUE obj);

  public:
    Tomato();
    ~Tomato();
    VALUE instance() { return this->rb_instance; }
    Handle<Object> global() { return context()->global(); }

    void rb_gc_mark();
    TomatoContext *context();
    void bind(VALUE object, const char *object_name);
};

extern "C" void Init_tomato(void);
extern VALUE cTomato;
extern VALUE cTomatoError;

#endif//TOMATO_H
