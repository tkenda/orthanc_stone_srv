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


import array
import matplotlib.pyplot as plt

def GenerateColormap(name):
    colormap = []

    for gray in range(256):
        if name == 'red':
            color = (gray / 255.0, 0, 0)
        elif name == 'green':
            color = (0, gray / 255.0, 0)
        elif name == 'blue':
            color = (0, 0, gray / 255.0)
        else:
            color = plt.get_cmap(name) (gray)

        colormap += map(lambda k: int(round(color[k] * 255)), range(3))

    #colormap[0] = 0
    #colormap[1] = 0
    #colormap[2] = 0

    return array.array('B', colormap).tostring()


for name in [ 
        'hot', 
        'jet', 
        'blue',
        'green',
        'red',
]:
    with open('%s.lut' % name, 'w') as f:
        f.write(GenerateColormap(name))
