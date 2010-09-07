require 'mkmf'
require 'set'

EXTERNAL = File.expand_path(File.dirname(__FILE__) + "/external")
BUILD = "#{EXTERNAL}/build/v8"

# I set ENV['FAST'] in the rakefile so I can skip building V8 when I rebuild
# Tomato.
puts
if !ENV['FAST'] || !File.exist?(BUILD)
  puts "Now compiling the V8 library..."
  system("cd \"#{EXTERNAL}\" && make") or raise "Error compiling V8!"
  puts "...done."
  puts
else
  puts "Skipping the build of V8 library -- you already have it."
  puts
end

find_header('v8.h', "#{BUILD}/include")
have_library('pthread')
have_library('objc') if RUBY_PLATFORM =~ /darwin/

$CPPFLAGS += " -Wall" unless $CPPFLAGS.split.include? "-Wall"
$CPPFLAGS += " -g" unless $CPPFLAGS.split.include? "-g"
$CPPFLAGS += " -rdynamic" unless $CPPFLAGS.split.include? "-rdynamic"

$DEFLIBPATH.unshift(BUILD)
$LIBS << ' -lv8'
find_header('dry.h', File.join(File.dirname(__FILE__), "include"))

CONFIG['LDSHARED'] = '$(CXX) -shared' unless RUBY_PLATFORM =~ /darwin/

create_makefile('tomato')

# now add a few extra targets
File.open("Makefile", "a") do |makefile|
makefile.print <<EOF

test: all
	@echo Running specs...
	spec spec -c

EOF
end
