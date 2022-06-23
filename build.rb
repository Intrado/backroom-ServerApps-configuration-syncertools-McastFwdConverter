#!/usr/bin/env ruby

require 'fileutils'
require '../gitscripts/CompileEnvDef.rb'

def execCmd(cmd)
  begin
    puts ":> " + cmd
    raise if not system(cmd)  
  rescue
    puts "Command FAILED: exiting ..."
    exit 1
  end
end

# build command variable
buildCmd = ["#{$devenv12} McastFwdConverter.sln  /rebuild \"Release\""]


# Files to be copied

toCopy = [
 %w(Release/McastFwdConverter.exe _bin),
 %w(Release/McastFwdConverter.pdb _bin),
 %w(McastFwd.xml.template _bin)
 ]


# execute all build command lines
buildCmd.each {|cmd| execCmd(cmd)}


# execute all copies
puts "\nCopying Files..."
toCopy.each {|file| 
  begin
    puts 'Copying ' + file[0] + ' to ' + file[1]
    FileUtils.mkdir_p(file[1])
    FileUtils.cp_r(file[0], file[1], :remove_destination => true) 
  rescue
    puts "ERROR: Failed to copy, exiting ..."
    exit 1
  end
}


