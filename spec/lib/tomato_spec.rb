require 'spec_helper'

describe Tomato do
  subject { Tomato.new }

  it "should build an anonymous Ruby object to mirror anonymous JS objects" do
    pending
  end
  
  it "should convert JS regexps to Ruby ones" do 
    pending "api support"
    subject.run("/./").should == /./
  end
  
  it "should raise ArgumentError given too few arguments to #run" do
    proc { Tomato.new.run }.should raise_error(ArgumentError)
  end
  
  it "should raise ArgumentError given too many arguments to #run" do
    proc { Tomato.new.run(1, 2, 3) }.should raise_error(ArgumentError)
  end
  
  it "should raise an error calling a missing method" do
    proc { subject.run("(nothing());") }.should raise_error(Tomato::Error)
  end
  
  it "should raise an error given bad syntax" do
    proc { subject.run("1a = 3b5;") }.should raise_error(Tomato::Error)
  end

  it 'should execute basic javascript' do
    # Basically, this demonstrates that initialization of the engine and actually *running* crap works.
    # Additionally, "(1+1);" is valid JavaScript so should not raise any other form of error.
      proc { subject.run("(1+1);").to_s.should == '2' }.should_not raise_error
  end
  
  it 'should return nil if result is null' do
      subject.run("(null);").should be_nil
  end

  it "should return :undefined if result is undefined" do
      subject.run("({}.x);").should == :undefined
  end

  it "should return true if result is true" do
      subject.run("(true);").should == true
  end

  it "should return false if result is true" do
      subject.run("(false);").should == false
  end
  
  it "should return a string if the result is a string" do
      subject.run("('hi');").should == "hi"
  end
  
  it "should return a string if the result is a function" do
      subject.run("(function() { });").should == "function () { }"
  end
  
  it "should return an array if the result is an array" do
      subject.run("([1, 2, 3]);").should == [1, 2, 3]
  end
  
  it "should return a Fixnum if the result is an integer" do
      subject.run("(1);").should == 1
      subject.run("(1);").should be_kind_of(Fixnum)
  end
  
  it "should return a Float if the result is a float or double" do
      subject.run("(1.05);").should == 1.05
      subject.run("(1.05);").should be_kind_of(Float)
  end
  
  it "should return a Date if the result is a date" do
    # JavaScript doesn't measure time in microseconds like ruby does, so we have to convert to milliseconds and lose
    # some precision in the process.
    time_in_millis = (Time.now.to_f * 1000.0).to_i
    code = "(new Date(#{time_in_millis}));"
    (subject.run(code).to_f * 1000.0).to_i.should == time_in_millis
  end
  
  it "should handle 64 bit integers" do
    subject.run("(9223372036854775806);").should == 9223372036854775806
#    /* TODO FIXME: There's no way to know if we're facing a Uint64. It works as a Float but user may not expect a Float... */
#    subject.run("(9223372036854775806);").should_not be_kind_of(Float) # and should be a Bignum, if we're feeling picky.
  end     
end
