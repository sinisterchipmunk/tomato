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

#include "debug.h"
#include "dry.h"
#include "errors.h"
#include "tomato_context.h"
#include "wrappers.h"
#include "reference_manager.h"

/* tomato.cpp */
class Tomato
{
  private:
    VALUE rb_instance;
    ReferenceManager references;

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
