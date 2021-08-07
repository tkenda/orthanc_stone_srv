#!/usr/bin/python

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
import pprint

init_printing(use_unicode=True)

s13, s23, s43 = symbols('s13 s23 s43')
x, y, z, w = symbols('x y z w')

A = Matrix([[ 1, 0, s13, 0 ],
            [ 0, 1, s23, 0 ],
            [ 0, 0, 1,   0 ],
            [ 0, 0, s43, 1 ]])

print('\nLacroute\'s shear matrix (A.14) is:')
pprint.pprint(A)

# At this point, we can write "print(A*p)". However, we don't care
# about the output "z" axis, as it is fixed. So we delete the 3rd row
# of A.

A.row_del(2)

p = Matrix([ x, y, z, 1 ])

v = A*p
print('\nAction of Lacroute\'s shear matrix on plane z (taking w=1):\n%s\n' % v)

print('Scaling = %s' % (1/v[2]))
print('Offset X = %s' % (v[0]/v[2]).subs(x, 0))
print('Offset Y = %s' % (v[1]/v[2]).subs(y, 0))
