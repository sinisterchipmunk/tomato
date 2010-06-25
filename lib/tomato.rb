$LOAD_PATH << File.expand_path("../../ext/tomato", __FILE__)
require 'tomato.so'

class Tomato
  def bind_method(method_name)
    _bind_method(method_name)
  end
  
  def inspect
    "#<Tomato>"
  end
end
