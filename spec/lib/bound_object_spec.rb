require 'spec_helper'

describe "Tomato bound objects" do
  subject { Tomato.new }
  
  it "should bind a Ruby class to an implicit chain" do
    now = Time.now.to_i
    
    subject.bind_object(Time)
    subject.run("Time.at(#{now})").should == Time.at(now)
  end

  it "should bind a Ruby object to an explicit chain" do
    time = Time.now
    subject.bind_object(time, "current_time")
    subject.run("current_time.to_s()").should == time.to_s
  end
  
  it "should bind a Ruby object's singleton methods" do
    time = Time.now
    def time.as_string; to_s; end
    time.should respond_to(:as_string)
    
    subject.bind_object(time, "current_time")
    subject.run("current_time.as_string()").should == time.to_s
  end
  
  it "should bind a Ruby object to an implicit chain under 'ruby.[unqualified_object_class_name]'" do
    time = Time.now
    subject.bind_object(time)
    subject.run("ruby.time.to_s()").should == time.to_s
  end
end
