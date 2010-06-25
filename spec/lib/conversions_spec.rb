require 'spec_helper'

describe "Tomato conversions" do
  context "from Ruby to JS to Ruby" do
    subject { Tomato.new }
    
    def handle(value)
      $test_value = value
      def subject.a_method; $test_value; end
      subject.bind_method(:a_method)
      subject.run("(a_method());")
    end
    
    it "should add a toString function to ruby symbols" do
      def subject.a_method; return :symbol; end
      subject.bind_method(:a_method)
      subject.run("a_method().toString();").should == "symbol"
    end
    
    it "should handle exceptions" do
      def subject.a_method; raise ArgumentError, "err"; end
      subject.bind_method(:a_method)
      proc { subject.run("(a_method());") }.should raise_error(ArgumentError)
    end
    
    it "should handle nil" do
      handle(nil).should == nil
    end
    
    it "should handle objects" do
      pending "how to do it? JSON?"
    end
    
    it "should handle classes" do
      pending "how to do it? Prototype?"
    end
    
    it "should handle modules" do
      pending "how to do it? Prototype?"
    end
    
    it "should handle floats" do
      handle(1.05).should == 1.05
    end
    
    it "should handle strings" do
      handle("hello").should == "hello"
    end
    
    it "should handle regexps" do
      pending "api support"
      handle(/./).should == /./
    end
    
    it "should handle arrays" do
      handle([1, 2, 3]).should == [1, 2, 3]
    end
    
    it "should handle hashes" do
      handle({:a => 1}).should == {:a => 1}
    end
    
    it "should handle structs" do
      pending
    end
    
    it "should handle bignums" do
      pending
    end
    
    it "should handle fixnum" do
      handle(1).should == 1
    end
    
    it "should handle true" do
      handle(true).should == true
    end
    
    it "should handle false" do
      handle(false).should == false
    end
    
    it "should handle undefined" do
      handle(:undefined).should == :undefined
    end
    
    it "should handle symbols" do
      handle(:symbol).should == :symbol
    end
  end
end