#ifndef DRY_H
#define DRY_H

/* for DRYness */
#define RB_EXTRACT_STRUCT(obj, klass, name)                            \
  klass *name;                                                         \
  Data_Get_Struct(obj, klass, name);

#define RB_ALLOC_FUNC(class_name) f##class_name##_allocate
#define RB_MARK_FUNC(class_name)  f##class_name##_mark
#define RB_FREE_FUNC(class_name)  f##class_name##_free

#define RB_DEFINE_ALLOC_FUNC(class_name)                                  \
  static VALUE RB_ALLOC_FUNC(class_name)(VALUE klass)                     \
  { /* klass is the same as class_name, so no need to use it directly. */ \
    class_name *obj = new class_name();                                   \
    return obj->instance();                                               \
  }
  
#define RB_DEFINE_MARK_FUNC(class_name)                                   \
  static void RB_MARK_FUNC(class_name)(class_name *obj)                   \
  {                                                                       \
    obj->rb_gc_mark();                                                    \
  }

#define RB_DEFINE_FREE_FUNC(class_name)                                   \
  static void RB_FREE_FUNC(class_name)(class_name *obj)                   \
  {                                                                       \
    delete obj;                                                           \
  }
  
#define RB_DEFINE_CLASS_FUNCS(class_name)                                 \
  RB_DEFINE_ALLOC_FUNC(class_name) \
  RB_DEFINE_MARK_FUNC(class_name)  \
  RB_DEFINE_FREE_FUNC(class_name)


// Extracts a C string from a V8 Utf8Value.
#define ToCString(value) (*value ? *value : "<string conversion failed>")

#define WRAP_CPP(x) try { x; }                                           \
                    catch(const std::string &s) { rb_raise(cTomatoError, s.c_str(), ""); } \
                    catch (const VALUE &exc) { rb_exc_raise(exc); }

#endif//DRY_H