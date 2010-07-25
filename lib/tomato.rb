unless defined?(Gem)
  require 'rubygems'
  gem 'sc-core-ext'
end

$LOAD_PATH << File.expand_path("../../ext/tomato", __FILE__)
require 'tomato.so'
require 'sc-core-ext'
require File.expand_path('../tomato/console', __FILE__)

class Tomato
  def inspect
    "#<Tomato>"
  end
end
