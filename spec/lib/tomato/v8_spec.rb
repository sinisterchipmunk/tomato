require 'spec_helper'

describe Tomato::V8 do
  it "should have the right V8 version" do
    Tomato::V8.version.should == "2.2.23"
  end
  
  it "should raise ArgumentError given too few arguments" do
    proc { Tomato.new.bind_object() }.should raise_error(ArgumentError)
  end
end
