require File.expand_path("../../lib/tomato", __FILE__)

module JavascriptAssertions
  def js_assert(a, b)
    proc do
      subject.run <<-end_js
        var a = #{a};
        var b = #{b};

        if (a != b) { throw " assertion failed: <"+a.toString()+"> is not equal to <"+b.toString()+">"; }
        true;
      end_js
    end.should_not raise_error
  end
end

Spec::Runner.configure do |config|
  config.include JavascriptAssertions
end
