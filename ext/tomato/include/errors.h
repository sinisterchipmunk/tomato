#ifndef ERRORS_H
#define ERRORS_H

/* errors.cpp */
extern VALUE WRAP_RB(VALUE (*meth)(VALUE), VALUE args);
extern void throw_error(TryCatch *try_catch);

#endif//ERRORS_H
