#!/usr/bin/env python

# Recursively read gtk jhbuild xml module files following include
# elements to produce the equivalent single xml file.

# N.B. This logic assumes exact syntax that is in gtk 3.4.1 version of
# module files including blank characters, and there is no guarantee
# for different gtk that that syntax will be immutable.  So it is likely
# this file will need maintenance for other versions of gtk.  See EXACT
# comments below where changes might have to be made.

# Copyright (C) 2013 Alan W. Irwin

# This file is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# This file is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with this file; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

import sys

def recursive_process(path, filename, if_included):
    fullpath = path + "/" + filename
    fileobject = open(fullpath)
    line = fileobject.readline()
    while line:
        if if_included:
            # ignore certain lines if the file is included
            # EXACT
            if line.find('<?xml version="1.0" ?>') >= 0:
                line = fileobject.readline()
                continue
            # EXACT
            if line.find('<moduleset>') >= 0:
                line = fileobject.readline()
                continue
            # EXACT
            if line.find('</moduleset>') >= 0:
                line = fileobject.readline()
                continue
        # EXACT
        index = line.find('<include href="')
        if index >=0:
            start = line.index('"')
            stop = line.rindex('"')
            included_filename = line[start+1: stop]
            recursive_process(path, included_filename, True)
        else:
            sys.stdout.write(line)
        line = fileobject.readline()
    fileobject.close()

#sys.setrecursionlimit(10)
fullpath = sys.argv[1]
index = fullpath.rfind("/")
if index >=0 and len(fullpath) > index + 1:
    path = fullpath[:index]
    filename = fullpath[index+1:]
elif index < 0 and len(fullpath) >= 1:
    path = "."
    filename = fullpath
else:
    print("%s is invalid fullpath name of a file" % fullpath)
    sys.exit()

recursive_process(path, filename, False)
