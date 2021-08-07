#!/usr/bin/env python

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


import subprocess
import sys

if len(sys.argv) <= 1:
    sys.stderr.write('Please provide arguments for uic\n')
    sys.exit(-1)

path = ''
pos = 1
while pos < len(sys.argv):
    if sys.argv[pos].startswith('-'):
        pos += 2
    else:
        path = sys.argv[pos]
        break

if len(path) == 0:
    sys.stderr.write('Unable to find the input file in the arguments to uic\n')
    sys.exit(-1)

with open(path, 'r') as f:
    lines = f.read().split('\n')
    if (len(lines) > 1 and
        lines[0].startswith('<?')):
        content = '\n'.join(lines[1:])
    else:
        content = '\n'.join(lines)
        
# Remove the source file from the arguments
args = sys.argv[1:pos] + sys.argv[pos+1:]

p = subprocess.Popen([ '/opt/lsb/bin/uic' ] + args,
                     stdin = subprocess.PIPE)
p.communicate(input = content)
