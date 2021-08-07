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
from sympy.solvers import solve
import pprint
import sys

init_printing(use_unicode=True)


# Create a test 3D vector using homogeneous coordinates
x, y, z, w = symbols('x y z w')
p = Matrix([ x, y, z, w ])


# Create a shear matrix, and a scale/shift "T * S" transform as in
# Lacroute's thesis (Equation A.16, page 209)
ex, ey, ew = symbols('ex ey ew')
sx, sy, tx, ty = symbols('sx sy tx ty')

TS = Matrix([[ sx, 0,  0, tx ],
             [ 0,  sy, 0, ty ],
             [ 0,  0,  1, 0  ],
             [ 0,  0,  0, 1  ]])

pureShear = Matrix([[ 1, 0, ex, 0 ],
                    [ 0, 1, ey, 0 ],
                    [ 0, 0, 1,  0 ],
                    [ 0, 0, ew, 1 ]])


# Create a general warp matrix, that corresponds to "M_warp" in
# Equation (A.17) of Lacroute's thesis:
ww11, ww12, ww13, ww14, ww21, ww22, ww23, ww24, ww31, ww32, ww33, ww34, ww41, ww42, ww43, ww44 = symbols('ww11 ww12 ww13 ww14 ww21 ww22 ww23 ww24 ww31 ww32 ww33 ww34 ww41 ww42 ww43 ww44')

WW = Matrix([[ ww11, ww12, ww13, ww14 ],
             [ ww21, ww22, ww23, ww24 ],
             [ ww31, ww32, ww33, ww34 ],
             [ ww41, ww43, ww43, ww44 ]])


# Create the matrix of intrinsic parameters of the camera
k11, k22, k14, k24 = symbols('k11 k22 k14 k24')
K = Matrix([[ k11, 0,   0, k14 ],
            [ 0,   k22, 0, k24 ],
            [ 0,   0,   0, 1   ]])


# The full decomposition is:
M_shear = TS * pureShear
M_warp = K * WW * TS.inv()
AA = M_warp * M_shear

# Check that the central component "M_warp == K * WW * TS.inv()" that
# is the left part of "A" is another general warp matrix (i.e. no
# exception is thrown about incompatible matrix sizes):
M_warp * p

if (M_warp.cols != 4 or
    M_warp.rows != 3):
    raise Exception('Invalid matrix size')


# We've just shown that "M_warp" is a general 3x4 matrix. Let's call
# it W:
w11, w12, w13, w14, w21, w22, w23, w24, w41, w42, w43, w44 = symbols('w11 w12 w13 w14 w21 w22 w23 w24 w41 w42 w43 w44')

W = Matrix([[ w11, w12, w13, w14 ],
            [ w21, w22, w23, w24 ],
            [ w41, w43, w43, w44 ]])

# This shows that it is sufficient to study a decomposition of the
# following form:
A = W * M_shear
print('\nA = W * M_shear =')
pprint.pprint(A)

sys.stdout.write('\nW = ')
pprint.pprint(W)

sys.stdout.write('\nM_shear = ')
pprint.pprint(M_shear)



# Let's consider one fixed 2D point (i,j) in the intermediate
# image. The 3D points (x,y,z,1) that are mapped to (i,j) must satisfy
# the equation "(i,j) == M_shear * (x,y,z,w)". As "M_shear" is
# invertible, we solve "(x,y,z,w) == inv(M_shear) * (i,j,k,1)".

i, j, k = symbols('i j k')
l = M_shear.inv() * Matrix([ i, j, k, 1 ])

print('\nLocus for points imaged to some fixed (i,j,k,l) point in the intermediate image:')
print('x = %s' % l[0])
print('y = %s' % l[1])
print('z = %s' % l[2])
print('w = %s' % l[3])


# By inspecting the 4 equations above, we see that the locus entirely
# depends upon the "k" value that encodes the Z-axis

print('\nGlobal effect of the shear-warp transform on this locus:')
q = expand(A * l)
pprint.pprint(q)

print("\nWe can arbitrarily fix the value of 'k', so let's choose 'k=0':")
pprint.pprint(q.subs(k, 0))

print("\nThis gives the warp transform.")
print("QED: line after Equation (A.17) on page 209.\n")
