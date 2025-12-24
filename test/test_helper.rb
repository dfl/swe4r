# frozen_string_literal: true

gem 'minitest'
require 'minitest/autorun'

$LOAD_PATH.unshift File.expand_path('../lib', __dir__)
require 'swe4r'

# Set ephemeris path for all tests
ENV['SE_EPHE_PATH'] = File.expand_path('../ext/swe4r', __dir__)
Swe4r.swe_set_ephe_path(ENV.fetch('SE_EPHE_PATH'))
