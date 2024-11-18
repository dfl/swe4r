require 'rake/testtask'

Rake::TestTask.new do |t|
  t.libs << 'test'
end

desc "Run tests"
task :default => :test

desc "build and install locally"
task :install do
  shell!('gem uninstall swe4r')
  shell!('gem build')
  latest = shell!('ls -t *.gem | head -n 1')
  shell!("gem install --local ./#{latest}")
  puts "done!"
end

def shell!(cmd)
  puts "---> #{cmd}"
  result = `#{cmd}`.chomp
  raise "Command failed: #{cmd}" unless $?.success?
  result
end
