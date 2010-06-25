require 'rubygems'
require 'rake'
require 'spec/rake/spectask'

begin
  require 'jeweler'
  Jeweler::Tasks.new do |gem|
    gem.name = "tomato"
    gem.summary = %Q{TODO: one-line summary of your gem}
    gem.description = %Q{TODO: longer description of your gem}
    gem.email = "sinisterchipmunk@gmail.com"
    gem.homepage = "http://github.com/sinisterchipmunk/tomato"
    gem.authors = ["Colin MacKenzie IV"]
    gem.add_development_dependency "rspec", ">= 1.3.0"
    # gem is a Gem::Specification... see http://www.rubygems.org/read/chapter/20 for additional settings
  end
  Jeweler::GemcutterTasks.new
rescue LoadError
  puts "Jeweler (or a dependency) not available. Install it with: gem install jeweler"
end

Spec::Rake::SpecTask.new(:test) do |test|
  test.libs << 'lib'
  test.pattern = 'spec/**/*_spec.rb'
  test.verbose = true
  test.spec_opts << ['-c']
end

begin
  require 'rcov/rcovtask'
  Rcov::RcovTask.new do |test|
    test.libs << 'test'
    test.pattern = 'test/**/test_*.rb'
    test.verbose = true
  end
rescue LoadError
  task :rcov do
    abort "RCov is not available. In order to run rcov, you must: sudo gem install spicycode-rcov"
  end
end

namespace :make do
  desc "Clean binaries"
  task :clean do
    chdir(File.expand_path("../ext/tomato", __FILE__)) do
      raise "Clean failed" unless system("make clean")
    end
  end
  
  desc "Build binaries"
  task :build do
    chdir(File.expand_path("../ext/tomato", __FILE__)) do
      unless system("gcc -MM *.cpp > depend") && system("ruby extconf.rb") && system("make all")
        raise "Build failed"
      end
    end
  end
end

task :test => ['make:build', :check_dependencies]

task :default => :test

require 'rake/rdoctask'
Rake::RDocTask.new do |rdoc|
  version = File.exist?('VERSION') ? File.read('VERSION') : ""

  rdoc.rdoc_dir = 'rdoc'
  rdoc.title = "tomato #{version}"
  rdoc.rdoc_files.include('README*')
  rdoc.rdoc_files.include('lib/**/*.rb')
end