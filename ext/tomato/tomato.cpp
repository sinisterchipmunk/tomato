#include "tomato.h"

static VALUE fTomato_run(int argc, VALUE *argv, VALUE self);
static VALUE fTomato_bind_object(int argc, VALUE *argv, VALUE self);
static VALUE fTomato_allocate(VALUE klass);
static void tomato_mark(Tomato *tomato);
static void tomato_free(Tomato *tomato);

VALUE cTomato;
VALUE cTomatoError;

Tomato::Tomato(VALUE klass)
{
  trace("create Tomato");
  this->rb_instance = Data_Wrap_Struct(klass, tomato_mark, tomato_free, this);
  this->rb_references = rb_hash_new();
  rb_iv_set(this->rb_instance, "@context", rb_class_new_instance(0, 0, cTomatoContext));
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

extern "C"
void Init_tomato(void)
{
  trace("Init_tomato");

  cTomato = rb_define_class("Tomato", rb_cObject);
  cTomatoError = rb_define_class_under(cTomato, "Error", rb_eRuntimeError);

  rb_define_alloc_func(cTomato, (VALUE(*)(VALUE))&fTomato_allocate);
  
  rb_define_method(cTomato, "run", (VALUE(*)(...))&fTomato_run, -1);
  rb_define_method(cTomato, "bind_object", (VALUE(*)(...))&fTomato_bind_object, -1);
  
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

TomatoContext *Tomato::context()
{
  TomatoContext *context;
  Data_Get_Struct(this->rb_instance, TomatoContext, context);
  return context;
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

/*
  call-seq:
    tomato.bind_object(5, "number")
    tomato.run("obj + 5") #=> 10
    
  Binds a Ruby object to this Tomato, making it accessible within this
  JavaScript context.
  
  Returns self.
*/
static VALUE fTomato_bind_object(int argc, VALUE *argv, VALUE self)
{
  if (argc != 2) rb_raise(rb_eArgError, "Expected 2 arguments");
  
  #ifdef DEBUG
    VALUE inspection = rb_funcall(argv[0], rb_intern("inspect"), 0);
    trace("Binding object as %s: %s", StringValuePtr(argv[1]), StringValuePtr(inspection));
  #endif
  
  Tomato *tomato;
  Data_Get_Struct(self, Tomato, tomato);
  
  tomato->context();
  
  return self;
}
