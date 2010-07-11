require 'spec_helper'

describe Tomato::V8 do
  it "should have the right V8 version" do
    Tomato::V8.version.should == "2.2.18"
  end
end
