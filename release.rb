#!/usr/bin/env ruby

# Ensure gitscripts are up to date
if not FileTest.directory?("../gitscripts")
	raise if not system('git clone git@github.com:Intrado/gitscripts.git ../gitscripts')
end
raise if not system('git -C ../gitscripts pull')
raise if not system('git -C ../gitscripts reset --hard HEAD')

# Define recipients, product, versionFiles and productFiles

recipients = ""


product = "McastFwdConverter"


versionFiles = ['McastFwdConverter.rc']


productFiles = [ # comma-separated files with relative path
]

# Release
require "../gitscripts/ReleaseProduct.rb"
releaseProduct(product, versionFiles, productFiles, recipients)


