Gem::Specification.new do |s|
  s.name              = 'swe4r'
  s.version           = '1.1.2'
  s.date              = '2024-11-10'
  s.summary           = "Swiss Ephemeris for Ruby (astrology)"
  s.description       = "Native bindings for the Swiss Ephemeris library (http://www.astro.com/swisseph/)"
  s.homepage          = "https://github.com/dfl/swe4r"
  s.author            = "David Lowenfels"
  s.email             = "dfl@alum.mit.edu"
  s.license           = "GPL-2.0-or-later"
  s.extra_rdoc_files  = ['README.rdoc']
  exclude_patterns = [
    'ext/swe4r/swemini.c',
    'ext/swe4r/sweasp.c',
    'ext/swe4r/swephgen4.c',
    'ext/swe4r/swetest.c',
    'ext/swe4r/swevents.c'
  ]
  s.files             = ( Dir.glob('lib/**/*.{rb}') + Dir.glob('ext/**/*.{rb,h,c}') ) - exclude_patterns
  s.extensions        = ['ext/swe4r/extconf.rb']
  s.required_ruby_version = ">= 2.0"
end
