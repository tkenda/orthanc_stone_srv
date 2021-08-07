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


import os
from PIL import Image

SOURCE = os.path.dirname(os.path.abspath(__file__))
TARGET = os.path.join(SOURCE, '..', 'WebApplication', 'img')

try:
    os.makedirs(TARGET)
except:  # Directory already exists
    pass
    
color = (217, 217, 217, 255)
border = 3
width = 32
height = 32



image = Image.new('RGBA', (width, height))

for x in range(0, width):
    for y in range(0, height):
        image.putpixel((x, y), color)

image.save(os.path.join(TARGET, 'grid1x1.png'), 'PNG')



image = Image.new('RGBA', (width, height))

for x in range(0, width / 2 - border):
    for y in range(0, height / 2 - border):
        image.putpixel((x, y), color)
    for y in range(height / 2 + border, height):
        image.putpixel((x, y), color)

for x in range(width / 2 + border, width):
    for y in range(0, height / 2 - border):
        image.putpixel((x, y), color)
    for y in range(height / 2 + border, height):
        image.putpixel((x, y), color)

image.save(os.path.join(TARGET, 'grid2x2.png'), 'PNG')



image = Image.new('RGBA', (width, height))

for y in range(0, height):
    for x in range(0, width / 2 - border):
        image.putpixel((x, y), color)
    for x in range(width / 2 + border, width):
        image.putpixel((x, y), color)

image.save(os.path.join(TARGET, 'grid2x1.png'), 'PNG')



image = Image.new('RGBA', (width, height))

for x in range(0, width):
    for y in range(0, height / 2 - border):
        image.putpixel((x, y), color)
    for y in range(height / 2 + border, height):
        image.putpixel((x, y), color)

image.save(os.path.join(TARGET, 'grid1x2.png'), 'PNG')
