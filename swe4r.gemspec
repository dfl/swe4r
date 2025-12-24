Gem::Specification.new do |s|
  s.name              = 'swe4r'
  s.version           = '1.2.0'
  s.date              = '2025-12-24'
  s.summary           = 'Swiss Ephemeris for Ruby (astrology)'
  s.description       = 'Native bindings for the Swiss Ephemeris library (http://www.astro.com/swisseph/)'
  s.homepage          = 'https://github.com/dfl/swe4r'
  s.author            = 'David Lowenfels'
  s.email             = 'dfl@alum.mit.edu'
  s.license           = 'GPL-2.0-or-later'
  s.extra_rdoc_files  = ['README.md']

  # Include only Ruby files and our wrapper C file + CMakeLists.txt
  # Swiss Ephemeris sources are fetched at build time via CMake
  s.files             = Dir.glob('lib/**/*.{rb}') +
                        ['ext/swe4r/extconf.rb',
                         'ext/swe4r/swe4r.c',
                         'ext/swe4r/CMakeLists.txt']

  s.extensions        = ['ext/swe4r/extconf.rb']
  s.required_ruby_version = '>= 2.6'

  # minitest 5.27+ requires Ruby >= 3.2, so constrain for Ruby 2.7 compatibility
  s.add_development_dependency 'minitest', '>= 5.16', '< 5.27'
  s.add_development_dependency 'minitest-reporters', '~> 1.6'
  s.add_development_dependency 'minitest-rg', '~> 5.3'
  s.add_development_dependency 'victor', '~> 0.3.4'

  s.metadata['rubygems_mfa_required'] = 'true'
  s.metadata['build_requirements'] = 'CMake >= 3.14'

  # Exclude fetched Swiss Ephemeris sources from RDoc to speed up gem installation
  s.rdoc_options = ['--exclude', 'swisseph_src']

  s.post_install_message = <<~MSG

    swe4r requires Swiss Ephemeris sources, which are automatically
    downloaded from GitHub during installation using CMake.

    If installation fails, ensure CMake 3.14+ is installed:
      - macOS: brew install cmake
      - Ubuntu/Debian: sudo apt-get install cmake
      - Or visit: https://cmake.org/download/

  MSG
end
