require "benchmark"
include Benchmark

require "fopen_struct"
require "fast_open_struct"
require "ostruct"

N = 5_000_000

puts "Dynamic attribute assignment:"

bmbm 36 do |b|
  b.report "OpenStruct" do
    os = OpenStruct.new
    N.times do
      os.a = 4
      os.b = 5
      os.c = 6
    end
  end

  b.report "FastOpenStruct" do
    os = FastOpenStruct.new
    N.times do
      os.a = 4
      os.b = 5
      os.c = 6
    end
  end

  b.report "FOpenStruct" do
    os = FOpenStruct.new
    N.times do
      os.a = 4
      os.b = 5
      os.c = 6
    end
  end

  FOpenStruct.calls_until_define_accessor = 10

  b.report "FOpenStruct with accessor definition" do
    os = FOpenStruct.new
    N.times do
      os.a = 4
      os.b = 5
      os.c = 6
    end
  end
end
