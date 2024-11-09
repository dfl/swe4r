require 'rake/testtask'

Rake::TestTask.new do |t|
  t.libs << 'test'
end

desc "Run tests"
task :default => :test

desc "build and install locally"
task :install do
  puts "uninstalling"
  `gem uninstall swe4r`
  puts "building"
  `gem build`
  latest = `ls -t *.gem | head -n 1`.chomp
  `gem install --local ./#{latest}`
  puts "done!"
end
