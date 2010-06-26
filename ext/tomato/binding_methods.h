#ifndef BINDING_METHODS_H
#define BINDING_METHODS_H

extern Handle<Value> bound_method(const Arguments& args);
extern int store_rb_message(const Arguments &args, V8Tomato **out_tomato, VALUE *out_receiver, ID *out_method_id);
extern void store_args(V8Tomato *tomato, VALUE rbargs, const Arguments &args);

#endif//BINDING_METHODS_H
