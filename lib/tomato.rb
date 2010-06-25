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
  def bind_method(method_name, *args)
    options = args.extract_options!
    receiver = options[:object] || args.first || self
    # Let C take it from here.
    receivers << receiver unless receivers.include?(receiver)
    _bind_method(method_name, receivers.index(receiver))
  end
  
  def inspect
    "#<Tomato>"
  end
  
  private
  def receivers
    @receivers ||= []
  end
end
