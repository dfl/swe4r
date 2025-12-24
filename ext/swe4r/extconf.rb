require 'mkmf'
require 'fileutils'

# Check if CMake is available
def cmake_available?
  system('cmake --version > /dev/null 2>&1')
end

# Run CMake to fetch Swiss Ephemeris sources
def fetch_swisseph_with_cmake
  puts "Fetching Swiss Ephemeris sources with CMake..."

  build_dir = File.join(__dir__, 'cmake_build')
  FileUtils.mkdir_p(build_dir)

  # Configure CMake
  unless system("cmake -S #{__dir__} -B #{build_dir}")
    raise "CMake configuration failed. Please ensure CMake 3.14+ is installed."
  end

  # Build (which triggers FetchContent)
  unless system("cmake --build #{build_dir}")
    raise "CMake build failed."
  end

  # Verify sources were fetched
  stamp_file = File.join(__dir__, '.swisseph_fetched')
  unless File.exist?(stamp_file)
    raise "Swiss Ephemeris sources not fetched successfully."
  end

  # Clean up build directory (keep fetched sources)
  FileUtils.rm_rf(build_dir)

  puts "Swiss Ephemeris sources fetched successfully."
end

# Check if Swiss Ephemeris sources need to be fetched
stamp_file = File.join(__dir__, '.swisseph_fetched')
swisseph_sources_exist = File.exist?(File.join(__dir__, 'sweph.c'))

if !swisseph_sources_exist || !File.exist?(stamp_file)
  unless cmake_available?
    raise <<~ERROR
      CMake is required to build this gem but was not found.

      Please install CMake 3.14 or later:
      - macOS: brew install cmake
      - Ubuntu/Debian: sudo apt-get install cmake
      - Or download from: https://cmake.org/download/
    ERROR
  end

  fetch_swisseph_with_cmake
end

# Get all C source files, excluding utility programs
$srcs = Dir.glob('*.c').select { |f|
  !['swephgen4.c', 'swemini.c', 'sweasp.c', 'swevents.c', 'swetest.c'].include?(f)
}

# Verify we have the necessary sources
if $srcs.empty? || !$srcs.include?('swe4r.c')
  raise "Required Swiss Ephemeris source files not found. Build cannot proceed."
end

create_makefile("swe4r/swe4r")