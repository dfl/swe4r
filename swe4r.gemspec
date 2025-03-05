Gem::Specification.new do |s|
  s.name              = 'swe4r'
  s.version           = '1.1.5'
  s.date              = '2025-03-05'
  s.summary           = 'Swiss Ephemeris for Ruby (astrology)'
  s.description       = 'Native bindings for the Swiss Ephemeris library (http://www.astro.com/swisseph/)'
  s.homepage          = 'https://github.com/dfl/swe4r'
  s.author            = 'David Lowenfels'
  s.email             = 'dfl@alum.mit.edu'
  s.license           = 'GPL-2.0-or-later'
  s.extra_rdoc_files  = ['README.rdoc']
  s.files             = Dir.glob('lib/**/*.{rb}') + Dir.glob('ext/**/*.{rb,h,c}')
  s.extensions        = ['ext/swe4r/extconf.rb']
  s.required_ruby_version = '>= 2.6'

  s.add_development_dependency 'minitest', '~> 5.16'
  s.add_development_dependency 'minitest-reporters', '~> 1.6'
  s.add_development_dependency 'minitest-rg', '~> 5.3'
  s.add_development_dependency 'victor', '~> 0.3.4'

  s.metadata['rubygems_mfa_required'] = 'true'
end
