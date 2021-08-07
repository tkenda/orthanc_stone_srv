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


# Setup "T * S * M_shear" (Equation A.16)

ex, ey, ew = symbols('ex ey ew')
sx, sy = symbols('sx, sy')
ti, tj = symbols('ti tj')

T = Matrix([[ 1, 0, 0, ti ],
            [ 0, 1, 0, tj ],
            [ 0, 0, 1, 0  ],
            [ 0, 0, 0, 1  ]])

# Equation (A.15), if "sx == sy == f"
S = Matrix([[ sx, 0,  0, 0 ],
            [ 0,  sy, 0, 0 ],
            [ 0,  0,  1, 0 ],
            [ 0,  0,  0, 1 ]])

# MM_shear, in Equation (A.14)
M = Matrix([[ 1, 0, ex, 0 ],
            [ 0, 1, ey, 0 ],
            [ 0, 0, 1,  0 ],
            [ 0, 0, ew,  1 ]])


x, y, z, w = symbols('x y z w')
p = Matrix([ x, y, z, w ])

print("\nT =" % T)
pprint.pprint(T);

print("\nS =" % T)
pprint.pprint(S);

print("\nM'_shear =" % T)
pprint.pprint(M);

print("\nGeneral form of a Lacroute's shear matrix (Equation A.16): T * S * M'_shear =")
pprint.pprint(T * S * M);

print("\nHence, alternative parametrization:")
a11, a13, a14, a22, a23, a24, a43 = symbols('a11 a13 a14 a22 a23 a24 a43')

A = Matrix([[ a11, 0,   a13, a14 ],
            [ 0,   a22, a23, a24 ],
            [ 0,   0,   1,   0   ],
            [ 0,   0,   a43, 1   ]])
pprint.pprint(A);

v = A * p
v = v.subs(w, 1)

print("\nAction of Lacroute's shear matrix A on plane z (taking w=1):\n%s\n" % v)

print('Output x\' = %s\n' % (v[0]/v[3]))
print('Output y\' = %s\n' % (v[1]/v[3]))
print('Output z\' = %s\n' % (v[2]/v[3]))
