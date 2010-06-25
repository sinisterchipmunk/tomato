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
    receivers << receiver unless receivers.include?(receiver)
    chain = options[:to] ? options[:to].split(/\./) : nil
    # Bind the method to JS.
    _bind_method(method_name, receivers.index(receiver), chain)
  end
  
  alias bind bind_method
  
  def inspect
    "#<Tomato>"
  end
  
  private
  def receivers
    @receivers ||= []
  end
end
