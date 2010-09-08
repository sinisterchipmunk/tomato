#include "tomato.h"

ReferenceManager::ReferenceManager()
{
  trace("create ReferenceManager<%x>", this);
  INC_TRACE_DEPTH;
  this->rb_references = rb_hash_new();
  DEC_TRACE_DEPTH;
}

ReferenceManager::~ReferenceManager()
{
  trace("destroy ReferenceManager<%x>", this);
}

VALUE ReferenceManager::reference(VALUE obj)
{
  VALUE object_id = rb_funcall(obj, rb_intern("object_id"), 0);
  VALUE ary = rb_hash_aref(this->rb_references, INT2FIX(object_id));
  if (NIL_P(ary))
  { /* if entry doesn't exist, create it */
    ary = rb_ary_new2(2);
    rb_hash_aset(this->rb_references, object_id, ary);
    rb_ary_store(ary, 0, INT2FIX(0));
    rb_ary_store(ary, 1, obj);
    #if DEBUG == 1
      VALUE inspection = rb_funcall(ary, rb_intern("inspect"), 0);
      trace("ReferenceManager<%x>#reference: creating entry %d => %s", this, NUM2LONG(object_id), StringValuePtr(inspection));
    #endif
  }
  return ary;
}

void ReferenceManager::add(VALUE obj)
{
  VALUE ref = this->reference(obj);
  int index = FIX2INT(rb_ary_entry(ref, 0)) + 1;
  rb_ary_store(ref, 0, INT2FIX(index));
  #if DEBUG == 1
    VALUE inspection = rb_funcall(ref, rb_intern("inspect"), 0);
    trace("ReferenceManager<%x>#add_reference - %s", this, StringValuePtr(inspection));
  #endif
}

void ReferenceManager::remove(VALUE obj)
{
//  VALUE ref = this->reference(obj);
//  if (rb_ary_entry(obj, 0) == 0)
//  {
//    rb_raise(rb_e)
//  }
}
