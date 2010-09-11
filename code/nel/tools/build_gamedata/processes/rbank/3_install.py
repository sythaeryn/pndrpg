#!/usr/bin/python
# 
# \file 3_install.py
# \brief Install rbank
# \date 2009-03-10-22-43-GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Install rbank
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
printLog(log, "--- Install rbank")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

printLog(log, ">>> Install rbank <<<")
clientPath = ClientDataDirectory + "/" + PacsClientDirectory
mkPath(log, clientPath)
srcPath = ExportBuildDirectory + "/" + RbankOutputBuildDirectory
mkPath(log, srcPath)
copyFilesNoTreeIfNeeded(log, srcPath, clientPath)
#clientPath = ClientDataDirectory + "/" + PacsClientDirectory
#mkPath(log, clientPath)
#srcPath = ExportBuildDirectory + "/" + RbankRetrieversBuildDirectory
#mkPath(log, srcPath)
#copyFileIfNeeded(log, srcPath + "/tempMerged.rbank", clientPath + "/" + RbankRbankName + ".rbank")
#copyFileIfNeeded(log, srcPath + "/tempMerged.gr", clientPath + "/" + RbankRbankName + ".gr")
#for file in findFiles(log, srcPath, "", ".lr"):
#	copyFileIfNeeded(log, srcPath + "/" + file, clientPath + "/" +  file.replace("tempMerged", RbankRbankName))
# mkPath(log, ExportBuildDirectory + "/" + rbankBuildDirectory)
# copyFilesNoTreeIfNeeded(log, ExportBuildDirectory + "/" + rbankBuildDirectory, clientPath)
#copyFileIfNeeded
printLog(log, "")

log.close()


# end of file
