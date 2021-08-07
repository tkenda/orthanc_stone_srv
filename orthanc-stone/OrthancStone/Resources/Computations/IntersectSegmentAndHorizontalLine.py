#!/usr/bin/env python

# Stone of Orthanc
# Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
# Department, University Hospital of Liege, Belgium
# Copyright (C) 2017-2021 Osimis S.A., Belgium
#
# This program is free software: you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program. If not, see
# <http://www.gnu.org/licenses/>.


from sympy import *

# Intersection between the 2D line segment (prevX,prevY)-(curX,curY) and the
# horizontal line "y = y0" using homogeneous coordinates

prevX, prevY, curX, curY, y0 = symbols('prevX prevY curX curY y0')

p1 = Matrix([prevX, prevY, 1])
p2 = Matrix([curX, curY, 1])
l1 = p1.cross(p2)

h1 = Matrix([0, y0, 1])
h2 = Matrix([1, y0, 1])
l2 = h1.cross(h2)

a = l1.cross(l2)

#pprint(cse(a/a[2], symbols = symbols('a b')))
pprint(a / a[2])