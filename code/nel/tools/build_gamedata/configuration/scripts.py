#!/usr/bin/python
# 
# \file export.py
# \brief Useful scripts
# \date 2009-02-18 09:22GMT
# \author Jan Boon (Kaetemi)
# Useful scripts
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

ActiveProjectDirectory = os.getenv("NELBUILDACTIVEPROJECT", "configuration/project")
sys.path.append(ActiveProjectDirectory)

def printLog(log, text):
	log.write(text + "\n")
	print text

def mkPath(log, path):
	printLog(log, "DIR " + path)
	distutils.dir_util.mkpath(path)

def needUpdate(log, source, dest):
	if (os.path.isfile(source)):
		if (os.path.isfile(dest)):
			if (os.stat(source).st_mtime > os.stat(dest).st_mtime):
				return 1
			else:
				return 0
		return 1
	printLog(log, "needUpdate: source doest not exist?! " + source)
	return 0

def needUpdateRemoveDest(log, source, dest):
	if (os.path.isfile(source)):
		if (os.path.isfile(dest)):
			if (os.stat(source).st_mtime > os.stat(dest).st_mtime):
				os.remove(dest)
				return 1
			else:
				return 0
		return 1
	printLog(log, "needUpdate: source doest not exist?! " + source)
	return 0

def needUpdateLogRemoveDest(log, source, dest):
	if (os.path.isfile(source)):
		if (os.path.isfile(dest)):
			if (os.stat(source).st_mtime > os.stat(dest).st_mtime):
				os.remove(dest)
				printLog(log, source + " -> " + dest)
				return 1
			else:
				printLog(log, "SKIP " + dest)
				return 0
		printLog(log, source + " -> " + dest)
		return 1
	printLog(log, "needUpdate: source doest not exist?! " + source)
	printLog(log, "SKIP " + dest)
	return 0

def copyFileList(log, dir_source, dir_target, files):
	for fileName in files:
		if fileName != ".svn":
			printLog(log, dir_source + "/" + fileName + " -> " + dir_target + "/" + fileName)
			shutil.copy(dir_source + "/" + fileName, dir_target + "/" + fileName)

def copyFileListNoTree(log, dir_source, dir_target, files):
	for fileName in files:
		if fileName != ".svn":
			printLog(log, dir_source + "/" + fileName + " -> " + dir_target + "/" + os.path.basename(fileName))
			shutil.copy(dir_source + "/" + fileName, dir_target + "/" + os.path.basename(fileName))

def copyFileListNoTreeIfNeeded(log, dir_source, dir_target, files):
	for fileName in files:
		if fileName != ".svn" and fileName != "*.*":
			srcFile = dir_source + "/" + fileName
			destFile = dir_target + "/" + os.path.basename(fileName)
			if needUpdateLogRemoveDest(log, srcFile, destFile):
				shutil.copy(srcFile, destFile)

def removeFilesRecursive(log, dir_files):
	files = os.listdir(dir_files)
	for fileName in files:
		if (fileName != ".svn"):
			if os.path.isdir(dir_files + "/" + fileName):
				removeFilesRecursive(log, dir_files + "/" + fileName)
			else:
				printLog(log, "RM " + dir_files + "/" + fileName)
				os.remove(dir_files + "/" + fileName)

def copyFilesRecursive(log, dir_source, dir_target):
	files = os.listdir(dir_source)
	mkPath(log, dir_target)
	for fileName in files:
		if (fileName != ".svn"):
			if os.path.isdir(dir_source + "/" + fileName):
				copyFilesRecursive(log, dir_source + "/" + fileName, dir_target + "/" + fileName)
			else:
				printLog(log, dir_source + "/" + fileName + " -> " + dir_target + "/" + fileName)
				shutil.copy(dir_source + "/" + fileName, dir_target + "/" + fileName)

def copyFiles(log, dir_source, dir_target):
	copyFileList(log, dir_source, dir_target, os.listdir(dir_source))

def copyFilesExt(log, dir_source, dir_target, file_ext):
	files = os.listdir(dir_source)
	len_file_ext = len(file_ext)
	for fileName in files:
		if (fileName != ".svn") and (fileName[-len_file_ext:].lower() == file_ext.lower()):
			printLog(log, dir_source + "/" + fileName + " -> " + dir_target + "/" + fileName)
			shutil.copy(dir_source + "/" + fileName, dir_target + "/" + fileName)

def copyFilesExtNoTree(log, dir_source, dir_target, file_ext):
	files = findFiles(log, dir_source, "", file_ext)
	copyFileListNoTree(log, dir_source, dir_target, files)

def copyFilesExtNoTreeIfNeeded(log, dir_source, dir_target, file_ext):
	files = findFiles(log, dir_source, "", file_ext)
	copyFileListNoTreeIfNeeded(log, dir_source, dir_target, files)

def copyFilesExtNoSubdirIfNeeded(log, dir_source, dir_target, file_ext):
	files = findFilesNoSubdir(log, dir_source, file_ext)
	copyFileListNoTreeIfNeeded(log, dir_source, dir_target, files)

def copyFilesNoTreeIfNeeded(log, dir_source, dir_target):
	copyFileListNoTreeIfNeeded(log, dir_source, dir_target, os.listdir(dir_source))

def copyFileListExtReplaceNoTreeIfNeeded(log, dir_source, dir_target, files, file_ext, target_ext):
	for fileName in files:
		if fileName != ".svn" and fileName != "*.*":
			srcFile = dir_source + "/" + fileName
			destFile = dir_target + "/" + os.path.basename(fileName)[0:-len(file_ext)] + target_ext
			if needUpdateLogRemoveDest(log, srcFile, destFile):
				shutil.copy(srcFile, destFile)
	
def copyFilesExtReplaceNoTreeIfNeeded(log, dir_source, dir_target, file_ext, target_ext):
	files = findFiles(log, dir_source, "", file_ext)
	copyFileListExtReplaceNoTreeIfNeeded(log, dir_source, dir_target, files, file_ext, target_ext)

def copyFileIfNeeded(log, srcFile, destFile):
	if needUpdateLogRemoveDest(log, srcFile, destFile):
		shutil.copy(srcFile, destFile)

def moveFileListNoTree(log, dir_source, dir_target, files):
	for fileName in files:
		if fileName != ".svn":
			printLog(log, dir_source + "/" + fileName + " -> " + dir_target + "/" + os.path.basename(fileName))
			shutil.move(dir_source + "/" + fileName, dir_target + "/" + os.path.basename(fileName))

def moveFilesExtNoTree(log, dir_source, dir_target, file_ext):
	files = findFiles(log, dir_source, "", file_ext)
	moveFileListNoTree(log, dir_source, dir_target, files)

def findFiles(log, dir_where, dir_sub, file_ext):
	result = [ ]
	files = os.listdir(dir_where + "/" + dir_sub)
	len_file_ext = len(file_ext)
	for fileName in files:
		if fileName != ".svn" and fileName != "*.*":
			filePath = dir_sub + fileName
			fileFull = dir_where + "/" + dir_sub + fileName
			if os.path.isfile(fileFull):
				if fileName[-len_file_ext:].lower() == file_ext.lower():
					result += [ filePath ]
			elif os.path.isdir(fileFull):
				result += findFiles(log, dir_where, filePath + "/", file_ext)
			else:
				printLog(log, "findFiles: file not dir or file?!" + filePath)
	return result

def findFilesNoSubdir(log, dir_where, file_ext):
	result = [ ]
	files = os.listdir(dir_where)
	len_file_ext = len(file_ext)
	for fileName in files:
		if fileName != ".svn" and fileName != "*.*":
			fileFull = dir_where + "/" + fileName
			if os.path.isfile(fileFull):
				if fileName[-len_file_ext:].lower() == file_ext.lower():
					result += [ fileName ]
			elif not os.path.isdir(fileFull):
				printLog(log, "findFilesNoSubdir: file not dir or file?!" + fileFull)
	return result

def findFile(log, dir_where, file_name):
	files = os.listdir(dir_where)
	for fileName in files:
		if fileName != ".svn" and fileName != "*.*":
			filePath = dir_where + "/" + fileName
			if os.path.isfile(filePath):
				if fileName == file_name:
					return filePath
			elif os.path.isdir(filePath):
				result = findFile(log, filePath, file_name)
				if result != "":
					return result
			else:
				printLog(log, "findFile: file not dir or file?! " + filePath)
	return ""

def findTool(log, dirs_where, file_name, suffix):
	try:
		for dir in dirs_where:
			tool = findFile(log, dir, file_name + suffix)
			if tool != "":
				printLog(log, "TOOL " + tool)
				return tool
	except Exception, e:
		printLog(log, "EXCEPTION " + str(e))
	printLog(log, "TOOL NOT FOUND " + file_name + suffix)
	return ""

def findMax(log, dir, file):
	tool = dir + "/" + file
	if os.path.isfile(tool):
		printLog(log, "3DSMAX " + tool)
		return tool
	printLog(log, "3DSMAX NOT FOUND " + file)
	return ""

def toolLogFail(log, tool, suffix):
	printLog(log, "FAIL " + tool + suffix + " is not found")

def askVar(log, name, default):
	sys.stdout.write(name + " (" + default + "): ")
	line = sys.stdin.readline()
	linestrip = line.strip()
	if linestrip == "--":
		log.write(name + " (" + default + "): ''\n")
		return ""
	elif linestrip == "":
		log.write(name + " (" + default + "): '" + default + "'\n")
		return default
	else:
		log.write(name + " (" + default + "): '" + linestrip + "'\n")
		return linestrip
