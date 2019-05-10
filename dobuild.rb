require 'p4'

begin
  p4 = P4.new()
  p4.connect()
  client = p4.fetch_client()
  p4.run_sync("-f", "//backroom/buildtools/...")
rescue P4Exception => msg
  puts( msg )
  p4.warnings.each { |w| puts( w ) }
  p4.errors.each { |e| puts( e ) }
 ensure
  p4.disconnect
end

require client['Root']+'/backroom/buildtools/buildtools'

$versionFile = ['McastFwdConverter.rc']

$project = "backroom/ServerApps/configuration/syncertools/McastFwdConverter"

$toCopy = [
 %w(Release/McastFwdConverter.exe _bin),
 %w(Release/McastFwdConverter.pdb _bin),
 %w(McastFwd.xml.template _bin)
 ]

$buildCmd = ["#{$devenv12} McastFwdConverter.sln  /rebuild \"Release\""]
$docCmd = [] 
$setupCmd = []

startBuild()
