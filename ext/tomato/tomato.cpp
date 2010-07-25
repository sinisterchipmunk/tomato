#include "tomato.h"

static VALUE fTomato_run(int argc, VALUE *argv, VALUE self);
static VALUE fTomato_allocate(VALUE klass);
static void tomato_mark(Tomato *tomato);
static void tomato_free(Tomato *tomato);

Tomato::Tomato(VALUE klass)
{
  trace("create Tomato");
  this->rb_instance = Data_Wrap_Struct(klass, tomato_mark, tomato_free, this);
  this->rb_references = rb_hash_new();
  rb_iv_set(this->rb_instance, "@context", rb_class_new_instance(0, 0, cV8));
}

Tomato::~Tomato()
{
  trace("destroy Tomato");
}

void Tomato::rb_gc_mark()
{
  ::rb_gc_mark(this->rb_instance);
  ::rb_gc_mark(this->rb_references);
}

VALUE cTomato;
VALUE cTomatoError;

extern "C"
void Init_tomato(void)
{
  trace("Init_tomato");

  cTomato = rb_define_class("Tomato", rb_cObject);
  cTomatoError = rb_define_class_under(cTomato, "Error", rb_eRuntimeError);

  rb_define_alloc_func(cTomato, (VALUE(*)(VALUE))&fTomato_allocate);
  
  rb_define_method(cTomato, "run", (VALUE(*)(...))&fTomato_run, -1);
  
  Init_v8();
  Init_debug();
}

static VALUE fTomato_allocate(VALUE klass)
{
  Tomato *tomato = new Tomato(klass);
  return tomato->instance();
}

static void tomato_mark(Tomato *tomato)
{
  tomato->rb_gc_mark();
}

static void tomato_free(Tomato *tomato)
{
  delete tomato;
}

/*
  call-seq:
    tomato.run("some javascript")

  Runs some JavaScript code and returns the result.
*/
static VALUE fTomato_run(int argc, VALUE *argv, VALUE self)
{
  return rb_funcall2(rb_iv_get(self, "@context"), rb_intern("run"), argc, argv);
}
