require 'spec_helper'

describe "Tomato bound objects" do
  subject { Tomato.new }
  
  class ::TestObject
    attr_accessor :i
    
    def initialize; @i = 1; end
    
    def ==(other)
      other.kind_of?(TestObject)
    end
  end
  
  it "should map attribute getters to corresponding Ruby methods" do
    subject.bind_object(TestObject, "TO")
    subject.run('var v = new TO(); v.i;').should == 1
  end
  
  it "should map attribute setters to corresponding Ruby methods" do
    subject.bind_object(TestObject, "TO")
    result = subject.run('var v = new TO(); v.i = 5; v;')
    result.i.should == 5
  end
  
  it "should be bind-able with an explicit chain" do
    subject.bind_object(TestObject, "TO")
    proc { subject.run("new TO();") }.should_not raise_error
  end
  
  it "should be instantiatable" do
    subject.bind_object(TestObject)
    subject.run("new TestObject();").should == TestObject.new
  end
  
  it "should not lose sub-objects" do
    subject.bind_object("hello", "TestObject.string")
    subject.bind_object(TestObject)
    subject.run("TestObject.string.inspect()").should == '"hello"'
  end
  
  it "should allow sub-bindings" do
    subject.bind_object(TestObject)
    subject.bind_object("hello", "TestObject.string")
    subject.run("TestObject.string.inspect()").should == '"hello"'
  end
end
