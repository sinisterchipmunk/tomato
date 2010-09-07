require 'spec_helper'

describe Tomato::Context do
  it "should have the right Context version" do
    Tomato::Context.version.should == "2.2.23"
  end
  
  it "should raise ArgumentError given too few arguments" do
    proc { Tomato.new.bind_object() }.should raise_error(ArgumentError)
  end
end
