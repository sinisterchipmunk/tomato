#ifndef BINDING_METHODS_H
#define BINDING_METHODS_H

#define TRY_JS(x)   try { x; }                                                                             \
                    catch(const std::string &msg) { return ThrowException(js_error_new(msg.c_str())); } 

extern Handle<Value> bound_method(const Arguments& args);
extern int store_rb_message(const Arguments &args, V8Tomato **out_tomato, VALUE *out_receiver, ID *out_method_id);
extern void store_args(V8Tomato *tomato, VALUE rbargs, const Arguments &args);

extern void tomatofy_function(Handle<Function> function, V8Tomato *tomato, VALUE receiver, Handle<Value> method_name);

#endif//BINDING_METHODS_H
