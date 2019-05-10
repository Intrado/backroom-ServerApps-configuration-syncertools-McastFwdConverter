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


$releaseDepot = "releases"

$p4Deps = ["backroom/Tools/EasyXerces/1.1.0.5/_export",
           "backroom/ServerApps/configuration/syncertools/SyncerHelper/1.0.0.31/_export"]

$svnDeps = []

# Main
print "Importing Project Dependencies...\n"
FileUtils.rm_rf("_import")
Dir.mkdir("_import")
getExports()
puts "==========================="
