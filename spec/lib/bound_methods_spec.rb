require 'spec_helper'

describe "Tomato bound methods" do
  subject { Tomato.new }
  
  it "should map Ruby methods to JavaScript methods" do
    subject.bind_method(:inspect).should == true
    proc { subject.run("(inspect());") }.should_not raise_error
  end
  
  it "should accept arguments" do
    subject.bind_method(:echo)
    def subject.echo(i); i; end
    subject.run("echo(1);").should == 1
  end
end
