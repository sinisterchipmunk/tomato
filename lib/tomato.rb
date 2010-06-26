$LOAD_PATH << File.expand_path("../../ext/tomato", __FILE__)
require 'tomato.so'
require 'sc-core-ext'

class Tomato
  # Bind a method to the JavaScript world. If no other arguments are given, the method will be called on this
  # instance of Tomato. For instance, the following code:
  #   t = Tomato.new
  #   t.bind_method(:inspect)
  #   t.run("inspect();")
  # ...will result in calling t.inspect.
  #
  # To bind the method to a specific object, pass the object itself in as a second argument or with an :object
  # option:
  #   t.bind_method(:inspect, my_funky_object)
  #   t.run("inspect();")
  #   # -or-
  #   t.bind_method(:inspect, :object => my_funky_object)
  #   t.run("inspect();")
  #   
  # Both of the above examples would result in a call to my_funky_object.inspect.
  #
  # You can also pass a :to option to bind the method to a specific object. If the specified object chain
  # does not exist, Tomato will generate generic objects to suit. Examples:
  # 
  #   t.bind_method(:inspect, my_funky_object, :to => "funky")
  #   t.run("funky.inspect();")
  # 
  #   t.bind_method(:inspect, my_funky_object, :to => "funky_objects.my_object")
  #   t.run("funky_objects.my_object.inspect();")
  #
  def bind_method(method_name, *args)
    options = args.extract_options!
    receiver = options[:object] || args.first || self
    chain = options[:to] ? split_chain(options[:to]) : nil
    # Bind the method to JS.
    _bind_method(method_name, receiver_index(receiver), chain)
  end
  
  # Binds an entire Ruby object to the specified JavaScript object chain.
  #
  # If the chain is omitted, and the object itself is a Class, then the chain will be the qualified name
  # of the class.
  #
  # Example:
  #  tomato.bind_object(Time)
  #  tomato.run("Time.now()")
  #   #=> 2010-06-25 18:02:52 -0400
  #
  # The same goes for Modules.
  #
  # Finally, if the chain is omitted and the object is an instance of a Class and not a Class itself, then
  # the object will be bound to "ruby.[unqualified_name]". If the unqualified name is already in use, it will
  # be overwritten.
  #
  # Example:
  #  time = Time.now
  #  tomato.bind_object(time)
  #  tomato.run("ruby.time.to_s()")
  #   #=> "2010-06-25 18:08:06 -0400"
  #  tomato.bind_object(time)
  #  tomato.run("ruby.time.to_s()")
  #   #=> "2010-06-25 18:08:29 -0400"
  def bind_object(obj, chain = nil)
    if (obj.kind_of?(Class) || obj.kind_of?(Module))
      if chain.nil?
        chain = obj.name.gsub(/\:\:/, '.')
        chain = chain[1..-1] if chain[0] == ?.
      end
      # This sets up an object chain with the last created object as a Function, which can
      # then be instiated with "new [Object]()" in JS.
      #
      # Objects of the same name in the chain are replaced, but their sub-objects are copied over
      # so it should be transparent to the user.
      return false unless _bind_class(receiver_index(obj), split_chain(chain))
    elsif chain.nil?
      unqualified_name = obj.class.name.gsub(/.*\:\:([^\:])?$/, '\1').underscore
      chain = "ruby.#{unqualified_name}"
    end
    
    obj.public_methods.each do |method_name|
      bind_method(method_name, obj, :to => chain)
    end
    obj
  end
  
  # Attempts to bind this object to the JavaScript world. If it's a String or Symbol, it will be treated
  # as a call to #bind_method; otherwise, a call to #bind_object.
  def bind(target, *args)
    if target.kind_of?(String) || target.kind_of?(Symbol)
      bind_method(target, *args)
    else
      bind_object(target, *args)
    end
  end
  
  def inspect
    "#<Tomato>"
  end
  
  private
  def split_chain(str)
    str.split(/\./)
  end
  
  def receivers
    @receivers ||= []
  end
  
  def receiver_index(receiver)
    receivers << receiver unless receivers.include?(receiver)
    receivers.index(receiver)
  end
end
