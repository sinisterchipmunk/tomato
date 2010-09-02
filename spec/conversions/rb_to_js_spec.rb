require 'spec_helper'

describe Tomato do
  subject { Tomato.new }

  it "should convert Fixnum to Int32" do
    assert_equal "1", 1
  end
end
