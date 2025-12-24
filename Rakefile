require 'rake/testtask'

Rake::TestTask.new do |t|
  t.libs << 'test'
end

desc 'Run tests'
task default: %i[install setup_test_files test]

desc 'Fetch Swiss Ephemeris files for testing'
task :setup_test_files do
  Dir.chdir('ext/swe4r') do
    unless File.exist?('.swisseph_fetched') && File.exist?('sweph.c')
      puts 'Fetching Swiss Ephemeris sources for testing...'
      system('cmake -S . -B cmake_build') or abort 'CMake configure failed'
      system('cmake --build cmake_build') or abort 'CMake build failed'
      system('rm -rf cmake_build')
    end
  end
end

desc 'build and install locally'
task :install do
  version = run_cmd('gem which swe4r')
  latest = run_cmd('ls -t *.gem | head -n 1')
  if version =~ /latest.chomp(".gem")/
    puts "already installed! #{latest}"
  else
    run_cmd('gem uninstall swe4r') unless version =~ /ERROR/m
    run_cmd('gem build swe4r.gemspec')
    latest = run_cmd('ls -t *.gem | head -n 1')
    run_cmd("gem install --local ./#{latest}")
  end
  puts 'done!'
end

def run_cmd(cmd)
  puts "----->  Running command: `#{cmd}`"
  `#{cmd}`.chomp
end
