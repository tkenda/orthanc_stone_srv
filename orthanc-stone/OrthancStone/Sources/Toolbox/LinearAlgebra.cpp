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


#include "LinearAlgebra.h"

#include "../StoneException.h"
#include "GenericToolbox.h"

#include <Logging.h>
#include <OrthancException.h>
#include <Toolbox.h>

#include <boost/lexical_cast.hpp>
#include <boost/numeric/ublas/lu.hpp>

#include <stdio.h>
#include <iostream>
#include <cstdlib>

namespace OrthancStone
{
  namespace LinearAlgebra
  {
    void Print(const Vector& v)
    {
      for (size_t i = 0; i < v.size(); i++)
      {
        printf("%g\n", v[i]);
        //printf("%8.2f\n", v[i]);
      }
      printf("\n");
    }


    void Print(const Matrix& m)
    {
      for (size_t i = 0; i < m.size1(); i++)
      {
        for (size_t j = 0; j < m.size2(); j++)
        {
          printf("%g  ", m(i,j));
          //printf("%8.2f  ", m(i,j));
        }
        printf("\n");        
      }
      printf("\n");        
    }


    bool ParseVector(Vector& target,
                     const std::string& value)
    {
      std::vector<std::string> items;
      Orthanc::Toolbox::TokenizeString(items, Orthanc::Toolbox::StripSpaces(value), '\\');

      target.resize(items.size());

      for (size_t i = 0; i < items.size(); i++)
      {
        /**
         * SJO - 2019-11-19 - WARNING: I reverted from "std::stod()"
         * to "boost::lexical_cast", as both "std::stod()" and
         * "std::strtod()" are sensitive to locale settings, making
         * this code non portable and very dangerous as it fails
         * silently. A string such as "1.3671875\1.3671875" is
         * interpreted as "1\1", because "std::stod()" expects a comma
         * (",") instead of a point ("."). This problem is notably
         * seen in Qt-based applications, that somehow set locales
         * aggressively.
         *
         * "boost::lexical_cast<>" is also dependent on the locale
         * settings, but apparently not in a way that makes this
         * function fail with Qt. The Orthanc core defines macro
         * "-DBOOST_LEXICAL_CAST_ASSUME_C_LOCALE" in static builds to
         * this end.
         **/
        
#if 0  // __cplusplus >= 201103L  // Is C++11 enabled?
        /**
         * We try and avoid the use of "boost::lexical_cast<>" here,
         * as it is very slow, and as Stone has to parse many doubles.
         * https://tinodidriksen.com/2011/05/cpp-convert-string-to-double-speed/
         **/
          
        try
        {
          target[i] = std::stod(items[i]);
        }
        catch (std::exception&)
        {
          // DO NOT use ".clear()" here, as the "Vector" class doesn't behave like std::vector!
          target.resize(0);
          return false;
        }

#elif 0
        /**
         * "std::strtod()" is the recommended alternative to
         * "std::stod()". It is apparently as fast as plain-C
         * "atof()", with more security.
         **/
        char* end = NULL;
        target[i] = std::strtod(items[i].c_str(), &end);
        if (end == NULL ||
            end != items[i].c_str() + items[i].size())
        {
          return false;
        }

#elif 1
        /**
         * Use of our homemade implementation of
         * "boost::lexical_cast<double>()". It is much faster than boost.
         **/
        if (!GenericToolbox::StringToDouble(target[i], items[i].c_str()))
        {
          return false;
        }
        
#else
        /**
         * Fallback implementation using Boost (slower, but somehow
         * independent to locale contrarily to "std::stod()", and
         * generic as it does not use our custom implementation).
         **/
        try
        {
          target[i] = boost::lexical_cast<double>(items[i]);
        }
        catch (boost::bad_lexical_cast&)
        {
          // DO NOT use ".clear()" here, as the "Vector" class doesn't behave like std::vector!
          target.resize(0);
          return false;
        }
#endif
      }

      return true;
    }


    bool ParseVector(Vector& target,
                     const Orthanc::DicomMap& dataset,
                     const Orthanc::DicomTag& tag)
    {
      std::string value;
      return (dataset.LookupStringValue(value, tag, false) &&
              ParseVector(target, value));
    }


    void NormalizeVector(Vector& u)
    {
      double norm = boost::numeric::ublas::norm_2(u);
      if (!IsCloseToZero(norm))
      {
        u = u / norm;
      }
    }

    void CrossProduct(Vector& result,
                      const Vector& u,
                      const Vector& v)
    {
      if (u.size() != 3 ||
          v.size() != 3)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
      }

      result.resize(3);

      result[0] = u[1] * v[2] - u[2] * v[1];
      result[1] = u[2] * v[0] - u[0] * v[2];
      result[2] = u[0] * v[1] - u[1] * v[0];
    }

    double DotProduct(const Vector& u, const Vector& v)
    {
      if (u.size() != 3 ||
          v.size() != 3)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
      }

      return (u[0] * v[0] + u[1] * v[1] + u[2] * v[2]);
    }

    void FillMatrix(Matrix& target,
                    size_t rows,
                    size_t columns,
                    const double values[])
    {
      target.resize(rows, columns);

      size_t index = 0;

      for (size_t y = 0; y < rows; y++)
      {
        for (size_t x = 0; x < columns; x++, index++)
        {
          target(y, x) = values[index];
        }
      }
    }


    void FillVector(Vector& target,
                    size_t size,
                    const double values[])
    {
      target.resize(size);

      for (size_t i = 0; i < size; i++)
      {
        target[i] = values[i];
      }
    }


    void Convert(Matrix& target,
                 const Vector& source)
    {
      const size_t n = source.size();

      target.resize(n, 1);

      for (size_t i = 0; i < n; i++)
      {
        target(i, 0) = source[i];
      }      
    }

    
    double ComputeDeterminant(const Matrix& a)
    {
      if (a.size1() != a.size2())
      {
        LOG(ERROR) << "Determinant only exists for square matrices";
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
      }
    
      // https://en.wikipedia.org/wiki/Rule_of_Sarrus
      if (a.size1() == 1)
      {
        return a(0,0);
      }
      else if (a.size1() == 2)
      {
        return a(0,0) * a(1,1) - a(0,1) * a(1,0);
      }
      else if (a.size1() == 3)
      {
        return (a(0,0) * a(1,1) * a(2,2) + 
                a(0,1) * a(1,2) * a(2,0) +
                a(0,2) * a(1,0) * a(2,1) -
                a(2,0) * a(1,1) * a(0,2) -
                a(2,1) * a(1,2) * a(0,0) -
                a(2,2) * a(1,0) * a(0,1));
      }
      else
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
      }
    }
    

    bool IsOrthogonalMatrix(const Matrix& q,
                            double threshold)
    {
      // https://en.wikipedia.org/wiki/Orthogonal_matrix

      if (q.size1() != q.size2())
      {
        LOG(ERROR) << "An orthogonal matrix must be squared";
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
      }

      using namespace boost::numeric::ublas;

      const Matrix check = prod(trans(q), q) - identity_matrix<double>(q.size1());

      type_traits<double>::real_type norm = norm_inf(check);

      return (norm <= threshold);
    }


    bool IsOrthogonalMatrix(const Matrix& q)
    {
      return IsOrthogonalMatrix(q, 10.0 * std::numeric_limits<float>::epsilon());
    }


    bool IsRotationMatrix(const Matrix& r,
                          double threshold)
    {
      return (IsOrthogonalMatrix(r, threshold) &&
              IsNear(ComputeDeterminant(r), 1.0, threshold));
    }


    bool IsRotationMatrix(const Matrix& r)
    {
      return IsRotationMatrix(r, 10.0 * std::numeric_limits<float>::epsilon());
    }


    void InvertUpperTriangularMatrix(Matrix& output,
                                     const Matrix& k)
    {
      if (k.size1() != k.size2())
      {
        LOG(ERROR) << "Determinant only exists for square matrices";
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
      }

      output.resize(k.size1(), k.size2());

      for (size_t i = 1; i < k.size1(); i++)
      {
        for (size_t j = 0; j < i; j++)
        {
          if (!IsCloseToZero(k(i, j)))
          {
            LOG(ERROR) << "Not an upper triangular matrix";
            throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
          }

          output(i, j) = 0;  // The output is also upper triangular
        }
      }

      if (k.size1() == 3)
      {
        // https://math.stackexchange.com/a/1004181
        double a = k(0, 0);
        double b = k(0, 1);
        double c = k(0, 2);
        double d = k(1, 1);
        double e = k(1, 2);
        double f = k(2, 2);

        if (IsCloseToZero(a) ||
            IsCloseToZero(d) ||
            IsCloseToZero(f))
        {
          LOG(ERROR) << "Singular upper triangular matrix";
          throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
        }
        else
        {
          output(0, 0) = 1.0 / a;
          output(0, 1) = -b / (a * d);
          output(0, 2) = (b * e - c * d) / (a * f * d);
          output(1, 1) = 1.0 / d;
          output(1, 2) = -e / (f * d);
          output(2, 2) = 1.0 / f;
        }
      }
      else
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
      }
    }


    static void GetGivensComponent(double& c,
                                   double& s,
                                   const Matrix& a,
                                   size_t i,
                                   size_t j)
    {
      assert(i < 3 && j < 3);

      double x = a(i, i);
      double y = a(i, j);
      double n = sqrt(x * x + y * y);

      if (IsCloseToZero(n))
      {
        c = 1;
        s = 0;
      }
      else
      {
        c = x / n;
        s = -y / n;
      }
    }


    /**
     * This function computes the RQ decomposition of a 3x3 matrix,
     * using Givens rotations. Reference: Algorithm A4.1 (page 579) of
     * "Multiple View Geometry in Computer Vision" (2nd edition).  The
     * output matrix "Q" is a rotation matrix, and "R" is upper
     * triangular.
     **/
    void RQDecomposition3x3(Matrix& r,
                            Matrix& q,
                            const Matrix& a)
    {
      using namespace boost::numeric::ublas;
      
      if (a.size1() != 3 ||
          a.size2() != 3)
      {
        LOG(ERROR) << "Only applicable to a 3x3 matrix";
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
      }

      r.resize(3, 3);
      q.resize(3, 3);

      r = a;
      q = identity_matrix<double>(3);

      {
        // Set A(2,1) to zero
        double c, s;
        GetGivensComponent(c, s, r, 2, 1);

        double v[9] = { 1, 0, 0, 
                        0, c, -s,
                        0, s, c };

        Matrix g;
        FillMatrix(g, 3, 3, v);

        r = prod(r, g);
        q = prod(trans(g), q);
      }


      {
        // Set A(2,0) to zero
        double c, s;
        GetGivensComponent(c, s, r, 2, 0);

        double v[9] = { c, 0, -s, 
                        0, 1, 0,
                        s, 0, c };

        Matrix g;
        FillMatrix(g, 3, 3, v);

        r = prod(r, g);
        q = prod(trans(g), q);
      }


      {
        // Set A(1,0) to zero
        double c, s;
        GetGivensComponent(c, s, r, 1, 0);

        double v[9] = { c, -s, 0, 
                        s, c, 0,
                        0, 0, 1 };

        Matrix g;
        FillMatrix(g, 3, 3, v);

        r = prod(r, g);
        q = prod(trans(g), q);
      }

      if (!IsCloseToZero(norm_inf(prod(r, q) - a)) ||
          !IsRotationMatrix(q) ||
          !IsCloseToZero(r(1, 0)) ||
          !IsCloseToZero(r(2, 0)) ||
          !IsCloseToZero(r(2, 1)))
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
      }
    }


    bool InvertMatrixUnsafe(Matrix& target,
                            const Matrix& source)
    {
      if (source.size1() != source.size2())
      {
        LOG(ERROR) << "Inverse only exists for square matrices";
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
      }

      if (source.size1() < 4)
      {
        // For matrices with size below 4, use direct computations
        // instead of LU decomposition

        if (source.size1() == 0)
        {
          // By convention, the inverse of the empty matrix, is itself the empty matrix
          target.resize(0, 0);
          return true;
        }

        double determinant = ComputeDeterminant(source);

        if (IsCloseToZero(determinant))
        {
          return false;
        }

        double denominator = 1.0 / determinant;

        target.resize(source.size1(), source.size2());

        if (source.size1() == 1)
        {
          target(0, 0) = denominator;

          return true;
        }
        else if (source.size1() == 2)
        {
          // https://en.wikipedia.org/wiki/Invertible_matrix#Inversion_of_2_%C3%97_2_matrices
          target(0, 0) =  source(1, 1) * denominator;
          target(0, 1) = -source(0, 1) * denominator;
          target(1, 0) = -source(1, 0) * denominator;
          target(1, 1) =  source(0, 0) * denominator;

          return true;
        }
        else if (source.size1() == 3)
        {
          // https://en.wikipedia.org/wiki/Invertible_matrix#Inversion_of_3_%C3%97_3_matrices
          const double a = source(0, 0);
          const double b = source(0, 1);
          const double c = source(0, 2);
          const double d = source(1, 0);
          const double e = source(1, 1);
          const double f = source(1, 2);
          const double g = source(2, 0);
          const double h = source(2, 1);
          const double i = source(2, 2);
        
          target(0, 0) =  (e * i - f * h) * denominator;
          target(0, 1) = -(b * i - c * h) * denominator;
          target(0, 2) =  (b * f - c * e) * denominator;
          target(1, 0) = -(d * i - f * g) * denominator;
          target(1, 1) =  (a * i - c * g) * denominator;
          target(1, 2) = -(a * f - c * d) * denominator;
          target(2, 0) =  (d * h - e * g) * denominator;
          target(2, 1) = -(a * h - b * g) * denominator;
          target(2, 2) =  (a * e - b * d) * denominator;

          return true;
        }
        else
        {
          throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
        }
      }
      else
      {
        // General case, using LU decomposition

        Matrix a = source;  // Copy the source matrix, as "lu_factorize()" modifies it

        boost::numeric::ublas::permutation_matrix<size_t>  permutation(source.size1());
        
        if (boost::numeric::ublas::lu_factorize(a, permutation) != 0)
        {
          return false;
        }
        else
        {
          target = boost::numeric::ublas::identity_matrix<double>(source.size1());
          lu_substitute(a, permutation, target);
          return true;
        }
      }
    }
    

    
    void InvertMatrix(Matrix& target,
                      const Matrix& source)
    {
      if (!InvertMatrixUnsafe(target, source))
      {
        LOG(ERROR) << "Cannot invert singular matrix";
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
      }
    }

    
    void CreateSkewSymmetric(Matrix& s,
                             const Vector& v)
    {
      if (v.size() != 3)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
      }

      s.resize(3, 3);
      s(0,0) = 0;
      s(0,1) = -v[2];
      s(0,2) = v[1];
      s(1,0) = v[2];
      s(1,1) = 0;
      s(1,2) = -v[0];
      s(2,0) = -v[1];
      s(2,1) = v[0];
      s(2,2) = 0;
    }

  
    Matrix InvertScalingTranslationMatrix(const Matrix& t)
    {
      if (t.size1() != 4 ||
          t.size2() != 4 ||
          !LinearAlgebra::IsCloseToZero(t(0,1)) ||
          !LinearAlgebra::IsCloseToZero(t(0,2)) ||
          !LinearAlgebra::IsCloseToZero(t(1,0)) ||
          !LinearAlgebra::IsCloseToZero(t(1,2)) ||
          !LinearAlgebra::IsCloseToZero(t(2,0)) ||
          !LinearAlgebra::IsCloseToZero(t(2,1)) ||
          !LinearAlgebra::IsCloseToZero(t(3,0)) ||
          !LinearAlgebra::IsCloseToZero(t(3,1)) ||
          !LinearAlgebra::IsCloseToZero(t(3,2)))
      {
        LOG(ERROR) << "This matrix is more than a zoom/translate transform";
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
      }

      const double sx = t(0,0);
      const double sy = t(1,1);
      const double sz = t(2,2);
      const double w = t(3,3);

      if (LinearAlgebra::IsCloseToZero(sx) ||
          LinearAlgebra::IsCloseToZero(sy) ||
          LinearAlgebra::IsCloseToZero(sz) ||
          LinearAlgebra::IsCloseToZero(w))
      {
        LOG(ERROR) << "Singular transform";
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
      }
    
      const double tx = t(0,3);
      const double ty = t(1,3);
      const double tz = t(2,3);
    
      Matrix m = IdentityMatrix(4);

      m(0,0) = 1.0 / sx;
      m(1,1) = 1.0 / sy;
      m(2,2) = 1.0 / sz;
      m(3,3) = 1.0 / w;

      m(0,3) = -tx / (sx * w);
      m(1,3) = -ty / (sy * w);
      m(2,3) = -tz / (sz * w);

      return m;
    }


    bool IsShearMatrix(const Matrix& shear)
    {
      return (shear.size1() == 4 &&
              shear.size2() == 4 &&
              LinearAlgebra::IsNear(1.0, shear(0,0)) &&
              LinearAlgebra::IsNear(0.0, shear(0,1)) &&
              LinearAlgebra::IsNear(0.0, shear(0,3)) &&
              LinearAlgebra::IsNear(0.0, shear(1,0)) &&
              LinearAlgebra::IsNear(1.0, shear(1,1)) &&
              LinearAlgebra::IsNear(0.0, shear(1,3)) &&
              LinearAlgebra::IsNear(0.0, shear(2,0)) &&
              LinearAlgebra::IsNear(0.0, shear(2,1)) &&
              LinearAlgebra::IsNear(1.0, shear(2,2)) &&
              LinearAlgebra::IsNear(0.0, shear(2,3)) &&
              LinearAlgebra::IsNear(0.0, shear(3,0)) &&
              LinearAlgebra::IsNear(0.0, shear(3,1)) &&
              LinearAlgebra::IsNear(1.0, shear(3,3)));
    }
  

    Matrix InvertShearMatrix(const Matrix& shear)
    {
      if (!IsShearMatrix(shear))
      {
        LOG(ERROR) << "Not a valid shear matrix";
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
      }

      Matrix m = IdentityMatrix(4);
      m(0,2) = -shear(0,2);
      m(1,2) = -shear(1,2);
      m(3,2) = -shear(3,2);

      return m;
    }
  }

  std::ostream& operator<<(std::ostream& s, const Vector& vec)
  {
    s << "(";
    for (size_t i = 0; i < vec.size(); ++i)
    {
      s << vec(i);
      if (i < (vec.size() - 1))
        s << ", ";
    }
    s << ")";
    return s;
  }

  std::ostream& operator<<(std::ostream& s, const Matrix& m)
  {
    ORTHANC_ASSERT(m.size1() == m.size2());
    s << "(";
    for (size_t i = 0; i < m.size1(); ++i)
    {
      s << "(";
      for (size_t j = 0; j < m.size2(); ++j)
      {
        s << m(i,j);
        if (j < (m.size2() - 1))
          s << ", ";
      }
      s << ")";
      if (i < (m.size1() - 1))
        s << ", ";
    }
    s << ")";
    return s;
  }
}
