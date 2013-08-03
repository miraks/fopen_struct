require "rake/testtask"

task default: :compile_and_test

task :compile do
  Dir.chdir File.expand_path("../ext/fopen_struct", __FILE__) do
    sh "ruby extconf.rb"
    sh "make"
  end
end

Rake::TestTask.new do |t|
  t.test_files = Dir["test/*.rb"]
end

task compile_and_test: [:compile, :test]

task irb: [:compile] do
  sh "ruby -I./lib -I./ext -rfopen_struct -rirb -e 'IRB.start'"
end

task bench: [:compile] do
  benchs = Dir["bench/*.rb"]
  benchs.each do |bench|
    sh "ruby #{bench}"
  end
end