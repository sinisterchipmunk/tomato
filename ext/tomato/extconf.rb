#require 'mkmf-rice'
#
##$LIBS << " -lstdc++"
#$CPPFLAGS << " -I/Users/colin/projects/gems/v8/v8-read-only/include"
#$LDFLAGS << " -L/Users/colin/projects/gems/v8/v8-read-only -lv8"
#
#have_header("v8.h")
#have_library("v8")#,  "v8::Handle")
#
##$LDFLAGS = "-lv8"
#create_makefile("tomato/tomato")
#

require 'mkmf'

$LIBS << " -lstdc++"
$CPPFLAGS << " -I/Users/colin/projects/gems/v8/v8-read-only/include"
$LDFLAGS << " -L/Users/colin/projects/gems/v8/v8-read-only -lv8"

have_header("v8.h")
have_library("v8")#,  "v8::Handle")

#$LDFLAGS = "-lv8"
create_makefile("tomato/tomato")
