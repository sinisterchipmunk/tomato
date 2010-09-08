require 'spec_helper'

describe Tomato do
  subject { Tomato.new }

  it "should convert Fixnum to Int32" do
    assert_equal "1", 1
  end
  
  it "should convert nil to null" do
    assert_equal "null", nil
  end
  
  it "should convert strings" do
    assert_equal "'test'", "test"
  end
  
  it "should convert floats" do
    assert_equal "1.7", 1.7
  end
  
  it "should convert regexps" do
    assert_equal "/1/", /1/
    pending "exposition in v8 api"
  end
  
  it "should convert arrays" do
    assert_equal "[1,2,3]", [1,2,3]
  end
end
