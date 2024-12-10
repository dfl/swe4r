require 'mkmf'
$srcs = Dir.glob('*.c').select { |f| 
  !['swephgen4.c', 'swemini.c', 'sweasp.c', 'swevents.c', 'swetest.c'].include?(f) 
}
create_makefile("swe4r/swe4r")