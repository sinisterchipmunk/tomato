#include "tomato.h"

static VALUE rb_reference_ary(VALUE ref)
{
  VALUE ary = rb_ary_new();
  rb_ary_push(ary, ref);
  rb_ary_push(ary, rb_funcall(ref, rb_intern("object_id"), 0));
  return ary;
}

void push_rb_reference(Tomato *tomato, VALUE ref)
{
  VALUE ary = rb_reference_ary(ref);
  int count = 0;
  VALUE rb_count = rb_hash_aref(tomato->rb_references, ary);
  if (!NIL_P(rb_count))
    count = FIX2INT(rb_count);
  rb_hash_aset(tomato->rb_references, ary, count+1);  
}

void pop_rb_reference(Tomato *tomato, VALUE ref)
{
  VALUE ary = rb_reference_ary(ref);
  int count;
  VALUE rb_count = rb_hash_aref(tomato->rb_references, ary);
  if (NIL_P(rb_count))
    throw std::string("Tomato: tried to remove a reference that didn't exist");
  count = FIX2INT(rb_count) - 1;
  if (count > 0)
    rb_hash_aset(tomato->rb_references, ary, count);
  else
    rb_hash_delete(tomato->rb_references, ary);
}
