/**
 * Stone of Orthanc
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017-2021 Osimis S.A., Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 **/


#pragma once

// Patch for ublas in Boost 1.64.0
// https://github.com/dealii/dealii/issues/4302
#include <boost/version.hpp>
#if BOOST_VERSION >= 106300  // or 64, need to check
#  include <boost/serialization/array_wrapper.hpp>
#endif

#include <DicomFormat/DicomMap.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

namespace OrthancStone
{
  typedef boost::numeric::ublas::matrix<double>   Matrix;
  typedef boost::numeric::ublas::vector<double>   Vector;

  // logs, debugging...
  std::ostream& operator<<(std::ostream& s, const Vector& vec);
  std::ostream& operator<<(std::ostream& s, const Matrix& m);

  namespace LinearAlgebra
  {
    void Print(const Vector& v);

    void Print(const Matrix& m);

    bool ParseVector(Vector& target,
                     const std::string& s);

    bool ParseVector(Vector& target,
                     const Orthanc::DicomMap& dataset,
                     const Orthanc::DicomTag& tag);

    inline void AssignVector(Vector& v,
                             double v1,
                             double v2)
    {
      v.resize(2);
      v[0] = v1;
      v[1] = v2;
    }


    inline void AssignVector(Vector& v,
                             double v1)
    {
      v.resize(1);
      v[0] = v1;
    }


    inline void AssignVector(Vector& v,
                             double v1,
                             double v2,
                             double v3)
    {
      v.resize(3);
      v[0] = v1;
      v[1] = v2;
      v[2] = v3;
    }


    inline void AssignVector(Vector& v,
                             double v1,
                             double v2,
                             double v3,
                             double v4)
    {
      v.resize(4);
      v[0] = v1;
      v[1] = v2;
      v[2] = v3;
      v[3] = v4;
    }


    inline Vector CreateVector(double v1)
    {
      Vector v;
      AssignVector(v, v1);
      return v;
    }

    
    inline Vector CreateVector(double v1,
                               double v2)
    {
      Vector v;
      AssignVector(v, v1, v2);
      return v;
    }

    
    inline Vector CreateVector(double v1,
                               double v2,
                               double v3)
    {
      Vector v;
      AssignVector(v, v1, v2, v3);
      return v;
    }

    
    inline Vector CreateVector(double v1,
                               double v2,
                               double v3,
                               double v4)
    {
      Vector v;
      AssignVector(v, v1, v2, v3, v4);
      return v;
    }


    inline bool IsNear(double x,
                       double y,
                       double threshold)
    {
      return fabs(x - y) <= threshold;
    }

    inline bool IsNear(double x,
                       double y)
    {
      // As most input is read as single-precision numbers, we take the
      // epsilon machine for float32 into consideration to compare numbers
      return IsNear(x, y, 10.0 * std::numeric_limits<float>::epsilon());
    }

    inline bool IsCloseToZero(double x)
    {
      return IsNear(x, 0.0);
    }

    void NormalizeVector(Vector& u);

    void CrossProduct(Vector& result,
                      const Vector& u,
                      const Vector& v);

    double DotProduct(const Vector& u, const Vector& v);

    void FillMatrix(Matrix& target,
                    size_t rows,
                    size_t columns,
                    const double values[]);

    void FillVector(Vector& target,
                    size_t size,
                    const double values[]);

    void Convert(Matrix& target,
                 const Vector& source);

    inline Matrix Transpose(const Matrix& a)
    {
      return boost::numeric::ublas::trans(a);
    }


    inline Matrix IdentityMatrix(size_t size)
    {
      return boost::numeric::ublas::identity_matrix<double>(size);
    }


    inline Matrix ZeroMatrix(size_t size1,
                             size_t size2)
    {
      return boost::numeric::ublas::zero_matrix<double>(size1, size2);
    }


    inline Matrix Product(const Matrix& a,
                          const Matrix& b)
    {
      return boost::numeric::ublas::prod(a, b);
    }


    inline Vector Product(const Matrix& a,
                          const Vector& b)
    {
      return boost::numeric::ublas::prod(a, b);
    }


    inline Matrix Product(const Matrix& a,
                          const Matrix& b,
                          const Matrix& c)
    {
      return Product(a, Product(b, c));
    }


    inline Matrix Product(const Matrix& a,
                          const Matrix& b,
                          const Matrix& c,
                          const Matrix& d)
    {
      return Product(a, Product(b, c, d));
    }


    inline Matrix Product(const Matrix& a,
                          const Matrix& b,
                          const Matrix& c,
                          const Matrix& d,
                          const Matrix& e)
    {
      return Product(a, Product(b, c, d, e));
    }


    inline Vector Product(const Matrix& a,
                          const Matrix& b,
                          const Vector& c)
    {
      return Product(Product(a, b), c);
    }


    inline Vector Product(const Matrix& a,
                          const Matrix& b,
                          const Matrix& c,
                          const Vector& d)
    {
      return Product(Product(a, b, c), d);
    }


    double ComputeDeterminant(const Matrix& a);

    bool IsOrthogonalMatrix(const Matrix& q,
                            double threshold);

    bool IsOrthogonalMatrix(const Matrix& q);

    bool IsRotationMatrix(const Matrix& r,
                          double threshold);

    bool IsRotationMatrix(const Matrix& r);

    void InvertUpperTriangularMatrix(Matrix& output,
                                     const Matrix& k); 

    /**
     * This function computes the RQ decomposition of a 3x3 matrix,
     * using Givens rotations. Reference: Algorithm A4.1 (page 579) of
     * "Multiple View Geometry in Computer Vision" (2nd edition).  The
     * output matrix "Q" is a rotation matrix, and "R" is upper
     * triangular.
     **/
    void RQDecomposition3x3(Matrix& r,
                            Matrix& q,
                            const Matrix& a);

    void InvertMatrix(Matrix& target,
                      const Matrix& source);

    // This is the same as "InvertMatrix()", but without exception
    bool InvertMatrixUnsafe(Matrix& target,
                            const Matrix& source);

    void CreateSkewSymmetric(Matrix& s,
                             const Vector& v);
  
    Matrix InvertScalingTranslationMatrix(const Matrix& t);

    bool IsShearMatrix(const Matrix& shear);  

    Matrix InvertShearMatrix(const Matrix& shear);
  };
}
