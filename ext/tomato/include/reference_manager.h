#ifndef REFERENCE_MANAGER_H
#define REFERENCE_MANAGER_H

class ReferenceManager
{
  private:
    // {object_id => reference(obj)}, e.g. {object_id => [1, obj]} 
    VALUE rb_references;
    
    /* Returns the array used as a value in rb_references, or a new array if none exists.
       The array contains [reference_count, obj] where reference_count is the number of open
       references to obj. */
    VALUE reference(VALUE obj);
  
  public:
    ReferenceManager();
    ~ReferenceManager();
    void rb_gc_mark() { ::rb_gc_mark(rb_references); }
    
    void add(VALUE obj);
    void remove(VALUE obj);
};

#endif//REFERENCE_MANAGER_H
