require 'spec_helper'

describe Tomato::Console do
  def output; @output ||= ""; end
  def run(code = ''); subject.process(code, output, @debug); output; end
  
  subject { Tomato::Console.new(StringIO.new(@input), StringIO.new(output), @debug) }
  before(:each) { @input = "" }
  
  it 'should switch to JS by default' do
    subject.input_mode.should == :js
  end
  
  it "should execute JS code on the tomato object" do
    run("(1+1);").should match(/ => 2/)
  end
  
  it "should switch to RB code" do
    run("rb").should match(/Now expecting Ruby code/)
  end
  
  it "should run RB code in the object namespace" do
    run("rb\ntomato").should match(/ => #<Tomato>/)
  end
  
  it "should recover all errors and return to console" do
    proc { run("this is a syntax error") }.should_not raise_error
  end
  
  it "should defer execution if line ends with a '\\'" do
    run("1 + \\ \n 2").should match(/ => 3/)
  end
end
