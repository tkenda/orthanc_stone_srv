#!/usr/bin/python

# Stone of Orthanc
# Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
# Department, University Hospital of Liege, Belgium
# Copyright (C) 2017-2021 Osimis S.A., Belgium
#
# This program is free software: you can redistribute it and/or
# modify it under the terms of the GNU Affero General Public License
# as published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.


#
# This maintenance script updates the content of the "Orthanc" folder
# to match the latest version of the Orthanc source code.
#

import multiprocessing
import os
import stat
import urllib2

TARGET = os.path.join(os.path.dirname(__file__), 'Orthanc')
PLUGIN_SDK_VERSION = '1.0.0'
REPOSITORY = 'https://hg.orthanc-server.com/orthanc/raw-file'

FILES = [
    ('OrthancServer/Plugins/Samples/Common/OrthancPluginCppWrapper.h',   'Plugins'),
    ('OrthancServer/Plugins/Samples/Common/OrthancPluginCppWrapper.cpp', 'Plugins'),
    ('OrthancServer/Plugins/Samples/Common/OrthancPluginException.h',    'Plugins'),
    ('OrthancServer/Plugins/Samples/Common/ExportedSymbolsPlugins.list', 'Plugins'),
    ('OrthancServer/Plugins/Samples/Common/OrthancPluginsExports.cmake', 'Plugins'),
    ('OrthancServer/Plugins/Samples/Common/VersionScriptPlugins.map',    'Plugins'),
]

SDK = [
    'orthanc/OrthancCPlugin.h',
]


def Download(x):
    branch = x[0]
    source = x[1]
    target = os.path.join(TARGET, x[2])
    print target

    try:
        os.makedirs(os.path.dirname(target))
    except:
        pass

    url = '%s/%s/%s' % (REPOSITORY, branch, source)

    with open(target, 'w') as f:
        f.write(urllib2.urlopen(url).read())


commands = []

for f in FILES:
    commands.append([ 'default',
                      f[0],
                      os.path.join(f[1], os.path.basename(f[0])) ])

for f in SDK:
    commands.append([
        'Orthanc-%s' % PLUGIN_SDK_VERSION, 
        'Plugins/Include/%s' % f,
        'Sdk-%s/%s' % (PLUGIN_SDK_VERSION, f) 
    ])

pool = multiprocessing.Pool(10)  # simultaneous downloads
pool.map(Download, commands)
