require 'spec_helper'

describe "Tomato bound methods" do
  subject { Tomato.new }
  
  it "should map Ruby methods to JavaScript methods" do
    subject.bind_method(:inspect).should == true
    proc { subject.run("(inspect());") }.should_not raise_error
  end
  
  it "should map Ruby methods to JS on a specific object within JS" do
    subject.bind_method(:inspect, "hello", :to => "greeting")
    subject.run("greeting.inspect();").should == '"hello"'
  end
  
  it "should map Ruby methods to JS on an object chain within JS" do
    subject.bind_method(:inspect, "hello", :to => "greeting.first")
    subject.run("greeting.first.inspect();").should == '"hello"'
  end
  
  it "should map Ruby methods to Javascript on a specific object without a hash" do
    subject.bind_method(:inspect, "hello").should == true
    subject.run("(inspect());").should == '"hello"'
  end
  
  it "should map Ruby methods to Javascript on a specific object with a hash" do
    subject.bind_method(:inspect, :object => "hello").should == true
    subject.run("(inspect());").should == '"hello"'
  end
  
  it "should accept arguments" do
    subject.bind_method(:echo)
    def subject.echo(i); i; end
    subject.run("echo(1);").should == 1
  end
end
