#!/usr/bin/python
# 
# \file 1_export.py
# \brief Export anim
# \date 2009-03-10 13:13GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Export anim
# 
# NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
# Copyright (C) 2010  Winch Gate Property Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
# 

import time, sys, os, shutil, subprocess, distutils.dir_util
sys.path.append("../../configuration")

if os.path.isfile("log.log"):
	os.remove("log.log")
log = open("log.log", "w")
from scripts import *
from buildsite import *
from process import *
from tools import *
from directories import *

printLog(log, "")
printLog(log, "-------")
printLog(log, "--- Export anim")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
Max = findMax(log, MaxDirectory, MaxExecutable)
printLog(log, "")

# For each anim directory
printLog(log, ">>> Export anim 3dsmax <<<")
mkPath(log, ExportBuildDirectory + "/" + AnimExportDirectory)
for dir in AnimSourceDirectories:
	mkPath(log, DatabaseDirectory + "/" + dir)
	if (needUpdateDirNoSubdirLogExtMultidir(log, DatabaseDirectory, AnimSourceDirectories, DatabaseDirectory + "/" + dir, ".max", ExportBuildDirectory + "/" + AnimExportDirectory, ".anim")):
		scriptSrc = "maxscript/anim_export.ms"
		scriptDst = MaxUserDirectory + "/scripts/anim_export.ms"
		logFile = ScriptDirectory + "/processes/anim/log.log"
		outDirAnim =  ExportBuildDirectory + "/" + AnimExportDirectory
		animSourceDir = DatabaseDirectory + "/" + dir
		tagList = findFiles(log, outDirAnim, "", ".anim")
		tagLen = len(tagList)
		if os.path.isfile(scriptDst):
			os.remove(scriptDst)
		tagDiff = 1
		sSrc = open(scriptSrc, "r")
		sDst = open(scriptDst, "w")
		for line in sSrc:
			newline = line.replace("output_logfile", logFile)
			newline = newline.replace("anim_source_directory", animSourceDir)
			newline = newline.replace("output_directory", outDirAnim)
			sDst.write(newline)
		sSrc.close()
		sDst.close()
		while tagDiff > 0:
			printLog(log, "MAXSCRIPT " + scriptDst)
			subprocess.call([ Max, "-U", "MAXScript", "anim_export.ms", "-q", "-mi", "-vn" ])
			tagList = findFiles(log, outDirAnim, "", ".anim")
			newTagLen = len(tagList)
			tagDiff = newTagLen - tagLen
			tagLen = newTagLen
			printLog(log, "Exported " + str(tagDiff) + " .anim files!")
		os.remove(scriptDst)

printLog(log, "")

log.close()


# end of file
