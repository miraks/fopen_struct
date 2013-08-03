require "fopen_struct/fopen_struct"

class FOpenStruct
  VERSION = "0.1.0"

  def inspect
    str = "#<#{apparent_class}"
     ids = (Thread.current[:__fopen_struct_inspect_key__] ||= Set.new)
     return str << " ...>" if ids.include? object_id
     ids << object_id
     begin
       first = true
       instance_variables.each do |ivar|
         str << "," unless first
         first = false
         str << " #{ivar[1..-1]}=#{instance_variable_get(ivar).inspect}"
       end
       str << ">"
     ensure
       ids.delete object_id
     end
  end
end