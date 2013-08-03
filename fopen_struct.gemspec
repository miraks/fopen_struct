Gem::Specification.new do |s|
  s.name          = "fopen_struct"
  s.version       = "0.1.0"
  s.authors       = ["Aleksey Volodkin"]
  s.email         = ["a@vldkn.net"]
  s.description   = %q{}
  s.summary       = %q{}
  s.homepage      = "https://github.com/miraks/fopen_struct"
  s.license       = "MIT"

  s.files = Dir.glob("lib/**/*.rb") + Dir.glob("ext/**/*.{c,h,rb}")
  s.test_files    = Dir.glob("spec/**/*_spec.rb")
  s.require_paths = ["lib", "ext"]
  s.extensions = ["ext/fopen_struct/extconf.rb"]

  s.required_ruby_version = ">= 1.9.2"

  s.add_development_dependency "rake"
  s.add_development_dependency "fast_open_struct"
end
