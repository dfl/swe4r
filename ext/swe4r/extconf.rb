require 'mkmf'

# List of source files to exclude
exclude_sources = %w[
  swemini.c
  sweasp.c
  swephgen4.c
  swetest.c
  swevents.c
]

# Collect all .c source files in the current directory
source_files = Dir.glob(File.join(__dir__, '*.c'))

# Exclude the unwanted source files
source_files.reject! { |f| exclude_sources.include?(File.basename(f)) }

# Set the object files to be compiled
$objs = source_files.map { |f| File.basename(f, '.c') + '.o' }

create_makefile('swe4r/swe4r')
