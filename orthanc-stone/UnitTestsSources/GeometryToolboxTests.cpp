/**
 * Stone of Orthanc
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017-2021 Osimis S.A., Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/


#include <gtest/gtest.h>

#include "../OrthancStone/Sources/Toolbox/DicomInstanceParameters.h"
#include "../OrthancStone/Sources/Toolbox/FiniteProjectiveCamera.h"
#include "../OrthancStone/Sources/Toolbox/GenericToolbox.h"
#include "../OrthancStone/Sources/Toolbox/GeometryToolbox.h"
#include "../OrthancStone/Sources/Toolbox/SlicesSorter.h"

#include <Logging.h>
#include <OrthancException.h>

#include <boost/math/special_functions/round.hpp>


TEST(GeometryToolbox, Interpolation)
{
  using namespace OrthancStone::GeometryToolbox;
  
  // https://en.wikipedia.org/wiki/Bilinear_interpolation#Application_in_image_processing
  ASSERT_DOUBLE_EQ(146.1f, ComputeBilinearInterpolationUnitSquare(0.5f, 0.2f, 91, 210, 162, 95));

  ASSERT_DOUBLE_EQ(91,  ComputeBilinearInterpolationUnitSquare(0, 0, 91, 210, 162, 95));
  ASSERT_DOUBLE_EQ(210, ComputeBilinearInterpolationUnitSquare(1, 0, 91, 210, 162, 95));
  ASSERT_DOUBLE_EQ(162, ComputeBilinearInterpolationUnitSquare(0, 1, 91, 210, 162, 95));
  ASSERT_DOUBLE_EQ(95,  ComputeBilinearInterpolationUnitSquare(1, 1, 91, 210, 162, 95));

  ASSERT_DOUBLE_EQ(123.35f, ComputeTrilinearInterpolationUnitSquare
                  (0.5f, 0.2f, 0.7f,
                   91, 210, 162, 95,
                   51, 190, 80, 92));

  ASSERT_DOUBLE_EQ(ComputeBilinearInterpolationUnitSquare(0.5f, 0.2f, 91, 210, 162, 95),
                  ComputeTrilinearInterpolationUnitSquare(0.5f, 0.2f, 0,
                                                          91, 210, 162, 95,
                                                          51, 190, 80, 92));

  ASSERT_DOUBLE_EQ(ComputeBilinearInterpolationUnitSquare(0.5f, 0.2f, 51, 190, 80, 92),
                  ComputeTrilinearInterpolationUnitSquare(0.5f, 0.2f, 1,
                                                          91, 210, 162, 95,
                                                          51, 190, 80, 92));
}


static bool CompareMatrix(const OrthancStone::Matrix& a,
                          const OrthancStone::Matrix& b,
                          double threshold = 0.00000001)
{
  if (a.size1() != b.size1() ||
      a.size2() != b.size2())
  {
    return false;
  }

  for (size_t i = 0; i < a.size1(); i++)
  {
    for (size_t j = 0; j < a.size2(); j++)
    {
      if (fabs(a(i, j) - b(i, j)) > threshold)
      {
        LOG(ERROR) << "Too large difference in component ("
                   << i << "," << j << "): " << a(i,j) << " != " << b(i,j);
        return false;
      }
    }
  }

  return true;
}


static bool CompareVector(const OrthancStone::Vector& a,
                          const OrthancStone::Vector& b,
                          double threshold = 0.00000001)
{
  if (a.size() != b.size())
  {
    return false;
  }

  for (size_t i = 0; i < a.size(); i++)
  {
    if (fabs(a(i) - b(i)) > threshold)
    {
      LOG(ERROR) << "Too large difference in component "
                 << i << ": " << a(i) << " != " << b(i);
      return false;
    }
  }

  return true;
}



TEST(FiniteProjectiveCamera, Decomposition1)
{
  // Example 6.2 of "Multiple View Geometry in Computer Vision - 2nd
  // edition" (page 163)
  const double p[12] = {
    3.53553e+2,  3.39645e+2,  2.77744e+2, -1.44946e+6,
    -1.03528e+2, 2.33212e+1,  4.59607e+2, -6.32525e+5,
    7.07107e-1,  -3.53553e-1, 6.12372e-1, -9.18559e+2
  };

  OrthancStone::FiniteProjectiveCamera camera(p);
  ASSERT_EQ(3u, camera.GetMatrix().size1());
  ASSERT_EQ(4u, camera.GetMatrix().size2());
  ASSERT_EQ(3u, camera.GetIntrinsicParameters().size1());
  ASSERT_EQ(3u, camera.GetIntrinsicParameters().size2());
  ASSERT_EQ(3u, camera.GetRotation().size1());
  ASSERT_EQ(3u, camera.GetRotation().size2());
  ASSERT_EQ(3u, camera.GetCenter().size());

  ASSERT_NEAR(1000.0, camera.GetCenter()[0], 0.01);
  ASSERT_NEAR(2000.0, camera.GetCenter()[1], 0.01);
  ASSERT_NEAR(1500.0, camera.GetCenter()[2], 0.01);

  ASSERT_NEAR(468.2, camera.GetIntrinsicParameters() (0, 0), 0.1);
  ASSERT_NEAR(91.2,  camera.GetIntrinsicParameters() (0, 1), 0.1);
  ASSERT_NEAR(300.0, camera.GetIntrinsicParameters() (0, 2), 0.1);
  ASSERT_NEAR(427.2, camera.GetIntrinsicParameters() (1, 1), 0.1);
  ASSERT_NEAR(200.0, camera.GetIntrinsicParameters() (1, 2), 0.1);
  ASSERT_NEAR(1.0,   camera.GetIntrinsicParameters() (2, 2), 0.1);

  ASSERT_NEAR(0, camera.GetIntrinsicParameters() (1, 0), 0.0000001);
  ASSERT_NEAR(0, camera.GetIntrinsicParameters() (2, 0), 0.0000001);
  ASSERT_NEAR(0, camera.GetIntrinsicParameters() (2, 1), 0.0000001);

  ASSERT_NEAR(0.41380,  camera.GetRotation() (0, 0), 0.00001);
  ASSERT_NEAR(0.90915,  camera.GetRotation() (0, 1), 0.00001);
  ASSERT_NEAR(0.04708,  camera.GetRotation() (0, 2), 0.00001);
  ASSERT_NEAR(-0.57338, camera.GetRotation() (1, 0), 0.00001);
  ASSERT_NEAR(0.22011,  camera.GetRotation() (1, 1), 0.00001);
  ASSERT_NEAR(0.78917,  camera.GetRotation() (1, 2), 0.00001);
  ASSERT_NEAR(0.70711,  camera.GetRotation() (2, 0), 0.00001);
  ASSERT_NEAR(-0.35355, camera.GetRotation() (2, 1), 0.00001);
  ASSERT_NEAR(0.61237,  camera.GetRotation() (2, 2), 0.00001);

  ASSERT_TRUE(OrthancStone::LinearAlgebra::IsRotationMatrix(camera.GetRotation()));

  OrthancStone::FiniteProjectiveCamera camera2(camera.GetIntrinsicParameters(),
                                               camera.GetRotation(),
                                               camera.GetCenter());

  ASSERT_TRUE(CompareMatrix(camera.GetMatrix(), camera2.GetMatrix()));
  ASSERT_TRUE(CompareMatrix(camera.GetIntrinsicParameters(), camera2.GetIntrinsicParameters()));
  ASSERT_TRUE(CompareMatrix(camera.GetRotation(), camera2.GetRotation()));
  ASSERT_TRUE(CompareVector(camera.GetCenter(), camera2.GetCenter()));
}


TEST(FiniteProjectiveCamera, Decomposition2)
{
  const double p[] = { 1188.111986, 580.205341, -808.445330, 128000.000000, -366.466264, 1446.510501, 418.499736, 128000.000000, -0.487118, 0.291726, -0.823172, 500.000000 };
  const double k[] = { -1528.494743, 0.000000, 256.000000, 0.000000, 1528.494743, 256.000000, 0.000000, 0.000000, 1.000000 };
  const double r[] = { -0.858893, -0.330733, 0.391047, -0.158171, 0.897503, 0.411668, -0.487118, 0.291726, -0.823172 };
  const double c[] = { 243.558936, -145.863085, 411.585964 };

  OrthancStone::FiniteProjectiveCamera camera(p);
  ASSERT_TRUE(OrthancStone::LinearAlgebra::IsRotationMatrix(camera.GetRotation()));

  OrthancStone::FiniteProjectiveCamera camera2(k, r, c);
  ASSERT_TRUE(CompareMatrix(camera.GetMatrix(), camera2.GetMatrix(), 1));
  ASSERT_TRUE(CompareMatrix(camera.GetIntrinsicParameters(), camera2.GetIntrinsicParameters(), 0.001));
  ASSERT_TRUE(CompareMatrix(camera.GetRotation(), camera2.GetRotation(), 0.000001));
  ASSERT_TRUE(CompareVector(camera.GetCenter(), camera2.GetCenter(), 0.0001));
}


TEST(FiniteProjectiveCamera, Decomposition3)
{
  const double p[] = { 10, 0, 0, 0,
                       0, 20, 0, 0,
                       0, 0, 30, 0 };

  OrthancStone::FiniteProjectiveCamera camera(p);
  ASSERT_TRUE(OrthancStone::LinearAlgebra::IsRotationMatrix(camera.GetRotation()));
  ASSERT_DOUBLE_EQ(10, camera.GetIntrinsicParameters() (0, 0));
  ASSERT_DOUBLE_EQ(20, camera.GetIntrinsicParameters() (1, 1));
  ASSERT_DOUBLE_EQ(30, camera.GetIntrinsicParameters() (2, 2));
  ASSERT_DOUBLE_EQ(1, camera.GetRotation() (0, 0));
  ASSERT_DOUBLE_EQ(1, camera.GetRotation() (1, 1));
  ASSERT_DOUBLE_EQ(1, camera.GetRotation() (2, 2));
  ASSERT_DOUBLE_EQ(0, camera.GetCenter() (0));
  ASSERT_DOUBLE_EQ(0, camera.GetCenter() (1));
  ASSERT_DOUBLE_EQ(0, camera.GetCenter() (2));
}


TEST(FiniteProjectiveCamera, Decomposition4)
{
  const double p[] = { 1, 0, 0, 10,
                       0, 1, 0, 20,
                       0, 0, 1, 30 };

  OrthancStone::FiniteProjectiveCamera camera(p);
  ASSERT_TRUE(OrthancStone::LinearAlgebra::IsRotationMatrix(camera.GetRotation()));
  ASSERT_DOUBLE_EQ(1, camera.GetIntrinsicParameters() (0, 0));
  ASSERT_DOUBLE_EQ(1, camera.GetIntrinsicParameters() (1, 1));
  ASSERT_DOUBLE_EQ(1, camera.GetIntrinsicParameters() (2, 2));
  ASSERT_DOUBLE_EQ(1, camera.GetRotation() (0, 0));
  ASSERT_DOUBLE_EQ(1, camera.GetRotation() (1, 1));
  ASSERT_DOUBLE_EQ(1, camera.GetRotation() (2, 2));
  ASSERT_DOUBLE_EQ(-10, camera.GetCenter() (0));
  ASSERT_DOUBLE_EQ(-20, camera.GetCenter() (1));
  ASSERT_DOUBLE_EQ(-30, camera.GetCenter() (2));
}


TEST(FiniteProjectiveCamera, Decomposition5)
{
  const double p[] = { 0, 0, 10, 0,
                       0, 20, 0, 0,
                       30, 0, 0, 0 };

  OrthancStone::FiniteProjectiveCamera camera(p);
  ASSERT_TRUE(OrthancStone::LinearAlgebra::IsRotationMatrix(camera.GetRotation()));
  ASSERT_DOUBLE_EQ(-10, camera.GetIntrinsicParameters() (0, 0));
  ASSERT_DOUBLE_EQ(20, camera.GetIntrinsicParameters() (1, 1));
  ASSERT_DOUBLE_EQ(30, camera.GetIntrinsicParameters() (2, 2));
  ASSERT_DOUBLE_EQ(-1, camera.GetRotation() (0, 2));
  ASSERT_DOUBLE_EQ(1, camera.GetRotation() (1, 1));
  ASSERT_DOUBLE_EQ(1, camera.GetRotation() (2, 0));
  ASSERT_DOUBLE_EQ(0, camera.GetCenter() (0));
  ASSERT_DOUBLE_EQ(0, camera.GetCenter() (1));
  ASSERT_DOUBLE_EQ(0, camera.GetCenter() (2));

  OrthancStone::FiniteProjectiveCamera camera2(camera.GetIntrinsicParameters(),
                                               camera.GetRotation(),
                                               camera.GetCenter());
  ASSERT_TRUE(CompareMatrix(camera.GetMatrix(), camera2.GetMatrix()));
  ASSERT_TRUE(CompareMatrix(camera.GetIntrinsicParameters(), camera2.GetIntrinsicParameters()));
  ASSERT_TRUE(CompareMatrix(camera.GetRotation(), camera2.GetRotation()));
  ASSERT_TRUE(CompareVector(camera.GetCenter(), camera2.GetCenter()));
}


static double GetCosAngle(const OrthancStone::Vector& a,
                          const OrthancStone::Vector& b)
{
  // Returns the cosine of the angle between two vectors
  // https://en.wikipedia.org/wiki/Dot_product#Geometric_definition
  return boost::numeric::ublas::inner_prod(a, b) / 
    (boost::numeric::ublas::norm_2(a) * boost::numeric::ublas::norm_2(b));
}


TEST(FiniteProjectiveCamera, Ray)
{
  const double pp[] = { -1499.650894, 2954.618773, -259.737419, 637891.819097,
                        -2951.517707, -1501.019129, -285.785281, 637891.819097,
                        0.008528, 0.003067, -0.999959, 2491.764918 };

  const OrthancStone::FiniteProjectiveCamera camera(pp);

  ASSERT_NEAR(-21.2492, camera.GetCenter() (0), 0.0001);
  ASSERT_NEAR(-7.64234, camera.GetCenter() (1), 0.00001);
  ASSERT_NEAR(2491.66, camera.GetCenter() (2), 0.01);

  // Image plane that led to these parameters, with principal point at
  // (256,256). The image has dimensions 512x512.
  OrthancStone::Vector o =
    OrthancStone::LinearAlgebra::CreateVector(7.009620, 2.521030, -821.942000);
  OrthancStone::Vector ax =
    OrthancStone::LinearAlgebra::CreateVector(-0.453219, 0.891399, -0.001131);
  OrthancStone::Vector ay =
    OrthancStone::LinearAlgebra::CreateVector(-0.891359, -0.453210, -0.008992);

  OrthancStone::CoordinateSystem3D imagePlane(o, ax, ay);

  // Back-projection of the principal point
  {
    OrthancStone::Vector ray = camera.GetRayDirection(256, 256);

    // The principal axis vector is orthogonal to the image plane
    // (i.e. parallel to the plane normal), in the opposite direction
    // ("-1" corresponds to "cos(pi)").
    ASSERT_NEAR(-1, GetCosAngle(ray, imagePlane.GetNormal()), 0.0000001);

    // Forward projection of principal axis, resulting in the principal point
    double x, y;
    camera.ApplyFinite(x, y, camera.GetCenter() - ray);

    ASSERT_NEAR(256, x, 0.00001);
    ASSERT_NEAR(256, y, 0.00001);
  }

  // Back-projection of the 4 corners of the image
  std::vector<double> cx, cy;
  cx.push_back(0);
  cy.push_back(0);
  cx.push_back(512);
  cy.push_back(0);
  cx.push_back(512);
  cy.push_back(512);
  cx.push_back(0);
  cy.push_back(512);

  bool first = true;
  double angle;

  for (size_t i = 0; i < cx.size(); i++)
  {
    OrthancStone::Vector ray = camera.GetRayDirection(cx[i], cy[i]);

    // Check that the angle wrt. principal axis is the same for all
    // the 4 corners
    double a = GetCosAngle(ray, imagePlane.GetNormal());
    if (first)
    {
      first = false;
      angle = a;
    }
    else
    {
      ASSERT_NEAR(angle, a, 0.000001);
    }

    // Forward projection of the ray, going back to the original point
    double x, y;
    camera.ApplyFinite(x, y, camera.GetCenter() - ray);
    
    ASSERT_NEAR(cx[i], x, 0.00001);
    ASSERT_NEAR(cy[i], y, 0.00001);

    // Alternative construction, by computing the intersection of the
    // ray with the image plane
    OrthancStone::Vector p;
    ASSERT_TRUE(imagePlane.IntersectLine(p, camera.GetCenter(), -ray));
    imagePlane.ProjectPoint(x, y, p);
    ASSERT_NEAR(cx[i], x + 256, 0.01);
    ASSERT_NEAR(cy[i], y + 256, 0.01);
  }
}


TEST(Matrix, Inverse1)
{
  OrthancStone::Matrix a, b;

  a.resize(0, 0);
  OrthancStone::LinearAlgebra::InvertMatrix(b, a);
  ASSERT_EQ(0u, b.size1());
  ASSERT_EQ(0u, b.size2());

  a.resize(2, 3);
  ASSERT_THROW(OrthancStone::LinearAlgebra::InvertMatrix(b, a), Orthanc::OrthancException);

  a.resize(1, 1);
  a(0, 0) = 45.0;

  ASSERT_DOUBLE_EQ(45, OrthancStone::LinearAlgebra::ComputeDeterminant(a));
  OrthancStone::LinearAlgebra::InvertMatrix(b, a);
  ASSERT_EQ(1u, b.size1());
  ASSERT_EQ(1u, b.size2());
  ASSERT_DOUBLE_EQ(1.0 / 45.0, b(0, 0));

  a(0, 0) = 0;
  ASSERT_DOUBLE_EQ(0, OrthancStone::LinearAlgebra::ComputeDeterminant(a));
  ASSERT_THROW(OrthancStone::LinearAlgebra::InvertMatrix(b, a), Orthanc::OrthancException);
}


TEST(Matrix, Inverse2)
{
  OrthancStone::Matrix a, b;
  a.resize(2, 2);
  a(0, 0) = 4;
  a(0, 1) = 3;
  a(1, 0) = 3;
  a(1, 1) = 2;

  ASSERT_DOUBLE_EQ(-1, OrthancStone::LinearAlgebra::ComputeDeterminant(a));
  OrthancStone::LinearAlgebra::InvertMatrix(b, a);
  ASSERT_EQ(2u, b.size1());
  ASSERT_EQ(2u, b.size2());

  ASSERT_DOUBLE_EQ(-2, b(0, 0));
  ASSERT_DOUBLE_EQ(3, b(0, 1));
  ASSERT_DOUBLE_EQ(3, b(1, 0));
  ASSERT_DOUBLE_EQ(-4, b(1, 1));

  a(0, 0) = 1;
  a(0, 1) = 2;
  a(1, 0) = 3;
  a(1, 1) = 4;

  ASSERT_DOUBLE_EQ(-2, OrthancStone::LinearAlgebra::ComputeDeterminant(a));
  OrthancStone::LinearAlgebra::InvertMatrix(b, a);

  ASSERT_DOUBLE_EQ(-2, b(0, 0));
  ASSERT_DOUBLE_EQ(1, b(0, 1));
  ASSERT_DOUBLE_EQ(1.5, b(1, 0));
  ASSERT_DOUBLE_EQ(-0.5, b(1, 1));
}


TEST(Matrix, Inverse3)
{
  OrthancStone::Matrix a, b;
  a.resize(3, 3);
  a(0, 0) = 7;
  a(0, 1) = 2;
  a(0, 2) = 1;
  a(1, 0) = 0;
  a(1, 1) = 3;
  a(1, 2) = -1;
  a(2, 0) = -3;
  a(2, 1) = 4;
  a(2, 2) = -2;

  ASSERT_DOUBLE_EQ(1, OrthancStone::LinearAlgebra::ComputeDeterminant(a));
  OrthancStone::LinearAlgebra::InvertMatrix(b, a);
  ASSERT_EQ(3u, b.size1());
  ASSERT_EQ(3u, b.size2());

  ASSERT_DOUBLE_EQ(-2, b(0, 0));
  ASSERT_DOUBLE_EQ(8, b(0, 1));
  ASSERT_DOUBLE_EQ(-5, b(0, 2));
  ASSERT_DOUBLE_EQ(3, b(1, 0));
  ASSERT_DOUBLE_EQ(-11, b(1, 1));
  ASSERT_DOUBLE_EQ(7, b(1, 2));
  ASSERT_DOUBLE_EQ(9, b(2, 0));
  ASSERT_DOUBLE_EQ(-34, b(2, 1));
  ASSERT_DOUBLE_EQ(21, b(2, 2));


  a(0, 0) = 1;
  a(0, 1) = 2;
  a(0, 2) = 2;
  a(1, 0) = 1;
  a(1, 1) = 0;
  a(1, 2) = 1;
  a(2, 0) = 1;
  a(2, 1) = 2;
  a(2, 2) = 1;

  ASSERT_DOUBLE_EQ(2, OrthancStone::LinearAlgebra::ComputeDeterminant(a));
  OrthancStone::LinearAlgebra::InvertMatrix(b, a);
  ASSERT_EQ(3u, b.size1());
  ASSERT_EQ(3u, b.size2());

  ASSERT_DOUBLE_EQ(-1, b(0, 0));
  ASSERT_DOUBLE_EQ(1, b(0, 1));
  ASSERT_DOUBLE_EQ(1, b(0, 2));
  ASSERT_DOUBLE_EQ(0, b(1, 0));
  ASSERT_DOUBLE_EQ(-0.5, b(1, 1));
  ASSERT_DOUBLE_EQ(0.5, b(1, 2));
  ASSERT_DOUBLE_EQ(1, b(2, 0));
  ASSERT_DOUBLE_EQ(0, b(2, 1));
  ASSERT_DOUBLE_EQ(-1, b(2, 2));
}


TEST(Matrix, Inverse4)
{
  OrthancStone::Matrix a, b;
  a.resize(4, 4);
  a(0, 0) = 2;
  a(0, 1) = 1;
  a(0, 2) = 2;
  a(0, 3) = -3;
  a(1, 0) = -2;
  a(1, 1) = 2;
  a(1, 2) = -1;
  a(1, 3) = -1;
  a(2, 0) = 2;
  a(2, 1) = 2;
  a(2, 2) = -3;
  a(2, 3) = -1;
  a(3, 0) = 3;
  a(3, 1) = -2;
  a(3, 2) = -3;
  a(3, 3) = -1;

  OrthancStone::LinearAlgebra::InvertMatrix(b, a);
  ASSERT_EQ(4u, b.size1());
  ASSERT_EQ(4u, b.size2());

  b *= 134.0;  // This is the determinant

  ASSERT_DOUBLE_EQ(8, b(0, 0));
  ASSERT_DOUBLE_EQ(-44, b(0, 1));
  ASSERT_DOUBLE_EQ(30, b(0, 2));
  ASSERT_DOUBLE_EQ(-10, b(0, 3));
  ASSERT_DOUBLE_EQ(2, b(1, 0));
  ASSERT_DOUBLE_EQ(-11, b(1, 1));
  ASSERT_DOUBLE_EQ(41, b(1, 2));
  ASSERT_DOUBLE_EQ(-36, b(1, 3));
  ASSERT_DOUBLE_EQ(16, b(2, 0));
  ASSERT_DOUBLE_EQ(-21, b(2, 1));
  ASSERT_DOUBLE_EQ(-7, b(2, 2));
  ASSERT_DOUBLE_EQ(-20, b(2, 3));
  ASSERT_DOUBLE_EQ(-28, b(3, 0));
  ASSERT_DOUBLE_EQ(-47, b(3, 1));
  ASSERT_DOUBLE_EQ(29, b(3, 2));
  ASSERT_DOUBLE_EQ(-32, b(3, 3));
}


TEST(FiniteProjectiveCamera, Calibration)
{
  unsigned int volumeWidth = 512;
  unsigned int volumeHeight = 512;
  unsigned int volumeDepth = 110;

  OrthancStone::Vector camera = OrthancStone::LinearAlgebra::CreateVector
    (-1000, -5000, -static_cast<double>(volumeDepth) * 32);

  OrthancStone::Vector principalPoint = OrthancStone::LinearAlgebra::CreateVector
    (volumeWidth/2, volumeHeight/2, volumeDepth * 2);

  OrthancStone::FiniteProjectiveCamera c(camera, principalPoint, 0, 512, 512, 1, 1);

  double swapv[9] = { 1, 0, 0,
                      0, -1, 512,
                      0, 0, 1 };
  OrthancStone::Matrix swap;
  OrthancStone::LinearAlgebra::FillMatrix(swap, 3, 3, swapv);

  OrthancStone::Matrix p = OrthancStone::LinearAlgebra::Product(swap, c.GetMatrix());
  p /= p(2,3);

  ASSERT_NEAR( 1.04437,     p(0,0), 0.00001);
  ASSERT_NEAR(-0.0703111,   p(0,1), 0.00000001);
  ASSERT_NEAR(-0.179283,    p(0,2), 0.000001);
  ASSERT_NEAR( 61.7431,     p(0,3), 0.0001);
  ASSERT_NEAR( 0.11127,     p(1,0), 0.000001);
  ASSERT_NEAR(-0.595541,    p(1,1), 0.000001);
  ASSERT_NEAR( 0.872211,    p(1,2), 0.000001);
  ASSERT_NEAR( 203.748,     p(1,3), 0.001);
  ASSERT_NEAR( 3.08593e-05, p(2,0), 0.0000000001);
  ASSERT_NEAR( 0.000129138, p(2,1), 0.000000001);
  ASSERT_NEAR( 9.18901e-05, p(2,2), 0.0000000001);
  ASSERT_NEAR( 1,           p(2,3), 0.0000001);
}


static bool IsEqualRotationVector(OrthancStone::Vector a,
                                  OrthancStone::Vector b)
{
  if (a.size() != b.size() ||
      a.size() != 3)
  {
    return false;
  }
  else
  {
    OrthancStone::LinearAlgebra::NormalizeVector(a);
    OrthancStone::LinearAlgebra::NormalizeVector(b);
    return OrthancStone::LinearAlgebra::IsCloseToZero(boost::numeric::ublas::norm_2(a - b));
  }
}


TEST(GeometryToolbox, AlignVectorsWithRotation)
{
  OrthancStone::Vector a, b;
  OrthancStone::Matrix r;

  OrthancStone::LinearAlgebra::AssignVector(a, -200, 200, -846.63);
  OrthancStone::LinearAlgebra::AssignVector(b, 0, 0, 1);

  OrthancStone::GeometryToolbox::AlignVectorsWithRotation(r, a, b);
  ASSERT_TRUE(OrthancStone::LinearAlgebra::IsRotationMatrix(r));
  ASSERT_TRUE(IsEqualRotationVector(OrthancStone::LinearAlgebra::Product(r, a), b));

  OrthancStone::GeometryToolbox::AlignVectorsWithRotation(r, b, a);
  ASSERT_TRUE(OrthancStone::LinearAlgebra::IsRotationMatrix(r));
  ASSERT_TRUE(IsEqualRotationVector(OrthancStone::LinearAlgebra::Product(r, b), a));

  OrthancStone::LinearAlgebra::AssignVector(a, 1, 0, 0);
  OrthancStone::LinearAlgebra::AssignVector(b, 0, 0, 1);
  OrthancStone::GeometryToolbox::AlignVectorsWithRotation(r, a, b);
  ASSERT_TRUE(OrthancStone::LinearAlgebra::IsRotationMatrix(r));
  ASSERT_TRUE(IsEqualRotationVector(OrthancStone::LinearAlgebra::Product(r, a), b));

  OrthancStone::LinearAlgebra::AssignVector(a, 0, 1, 0);
  OrthancStone::LinearAlgebra::AssignVector(b, 0, 0, 1);
  OrthancStone::GeometryToolbox::AlignVectorsWithRotation(r, a, b);
  ASSERT_TRUE(OrthancStone::LinearAlgebra::IsRotationMatrix(r));
  ASSERT_TRUE(IsEqualRotationVector(OrthancStone::LinearAlgebra::Product(r, a), b));

  OrthancStone::LinearAlgebra::AssignVector(a, 0, 0, 1);
  OrthancStone::LinearAlgebra::AssignVector(b, 0, 0, 1);
  OrthancStone::GeometryToolbox::AlignVectorsWithRotation(r, a, b);
  ASSERT_TRUE(OrthancStone::LinearAlgebra::IsRotationMatrix(r));
  ASSERT_TRUE(IsEqualRotationVector(OrthancStone::LinearAlgebra::Product(r, a), b));

  OrthancStone::LinearAlgebra::AssignVector(a, 0, 0, 0);
  OrthancStone::LinearAlgebra::AssignVector(b, 0, 0, 1);
  ASSERT_THROW(OrthancStone::GeometryToolbox::AlignVectorsWithRotation(r, a, b), Orthanc::OrthancException);

  // TODO: Deal with opposite vectors

  /*
    OrthancStone::LinearAlgebra::AssignVector(a, 0, 0, -1);
    OrthancStone::LinearAlgebra::AssignVector(b, 0, 0, 1);
    OrthancStone::GeometryToolbox::AlignVectorsWithRotation(r, a, b);
    OrthancStone::LinearAlgebra::Print(r);
    OrthancStone::LinearAlgebra::Print(boost::numeric::ublas::prod(r, a));
  */
}


static bool IsEqualVectorL1(OrthancStone::Vector a,
                            OrthancStone::Vector b)
{
  if (a.size() != b.size())
  {
    return false;
  }
  else
  {
    for (size_t i = 0; i < a.size(); i++)
    {
      if (!OrthancStone::LinearAlgebra::IsNear(a[i], b[i], 0.0001))
      {
        return false;
      }
    }

    return true;
  }
}


TEST(VolumeImageGeometry, Basic)
{
  using namespace OrthancStone;
  
  VolumeImageGeometry g;
  g.SetSizeInVoxels(10, 20, 30);
  g.SetVoxelDimensions(1, 2, 3);

  Vector p = g.GetCoordinates(0, 0, 0);
  ASSERT_EQ(3u, p.size());
  ASSERT_DOUBLE_EQ(-1.0 / 2.0, p[0]);
  ASSERT_DOUBLE_EQ(-2.0 / 2.0, p[1]);
  ASSERT_DOUBLE_EQ(-3.0 / 2.0, p[2]);
  
  p = g.GetCoordinates(1, 1, 1);
  ASSERT_DOUBLE_EQ(-1.0 / 2.0 + 10.0 * 1.0, p[0]);
  ASSERT_DOUBLE_EQ(-2.0 / 2.0 + 20.0 * 2.0, p[1]);
  ASSERT_DOUBLE_EQ(-3.0 / 2.0 + 30.0 * 3.0, p[2]);

  VolumeProjection proj;
  bool isOpposite;
  ASSERT_TRUE(g.DetectProjection(proj, isOpposite, g.GetAxialGeometry().GetNormal()));
  ASSERT_EQ(VolumeProjection_Axial, proj);
  ASSERT_FALSE(isOpposite);
  ASSERT_TRUE(g.DetectProjection(proj, isOpposite, g.GetCoronalGeometry().GetNormal()));
  ASSERT_EQ(VolumeProjection_Coronal, proj);
  ASSERT_FALSE(isOpposite);
  ASSERT_TRUE(g.DetectProjection(proj, isOpposite, g.GetSagittalGeometry().GetNormal()));
  ASSERT_EQ(VolumeProjection_Sagittal, proj);
  ASSERT_FALSE(isOpposite);

  ASSERT_EQ(10u, g.GetProjectionWidth(VolumeProjection_Axial));
  ASSERT_EQ(20u, g.GetProjectionHeight(VolumeProjection_Axial));
  ASSERT_EQ(30u, g.GetProjectionDepth(VolumeProjection_Axial));
  ASSERT_EQ(10u, g.GetProjectionWidth(VolumeProjection_Coronal));
  ASSERT_EQ(30u, g.GetProjectionHeight(VolumeProjection_Coronal));
  ASSERT_EQ(20u, g.GetProjectionDepth(VolumeProjection_Coronal));
  ASSERT_EQ(20u, g.GetProjectionWidth(VolumeProjection_Sagittal));
  ASSERT_EQ(30u, g.GetProjectionHeight(VolumeProjection_Sagittal));
  ASSERT_EQ(10u, g.GetProjectionDepth(VolumeProjection_Sagittal));

  p = g.GetVoxelDimensions(VolumeProjection_Axial);
  ASSERT_EQ(3u, p.size());
  ASSERT_DOUBLE_EQ(1, p[0]);
  ASSERT_DOUBLE_EQ(2, p[1]);
  ASSERT_DOUBLE_EQ(3, p[2]);
  p = g.GetVoxelDimensions(VolumeProjection_Coronal);
  ASSERT_EQ(3u, p.size());
  ASSERT_DOUBLE_EQ(1, p[0]);
  ASSERT_DOUBLE_EQ(3, p[1]);
  ASSERT_DOUBLE_EQ(2, p[2]);
  p = g.GetVoxelDimensions(VolumeProjection_Sagittal);
  ASSERT_EQ(3u, p.size());
  ASSERT_DOUBLE_EQ(2, p[0]);
  ASSERT_DOUBLE_EQ(3, p[1]);
  ASSERT_DOUBLE_EQ(1, p[2]);

  // Loop over all the voxels of the volume
  for (unsigned int z = 0; z < g.GetDepth(); z++)
  {
    const float zz = (0.5f + static_cast<float>(z)) / static_cast<float>(g.GetDepth());  // Z-center of the voxel
    
    for (unsigned int y = 0; y < g.GetHeight(); y++)
    {
      const float yy = (0.5f + static_cast<float>(y)) / static_cast<float>(g.GetHeight());  // Y-center of the voxel

      for (unsigned int x = 0; x < g.GetWidth(); x++)
      {
        const float xx = (0.5f + static_cast<float>(x)) / static_cast<float>(g.GetWidth());  // X-center of the voxel

        const float sx = 1.0f;
        const float sy = 2.0f;
        const float sz = 3.0f;
        
        Vector p = g.GetCoordinates(xx, yy, zz);

        Vector q = (g.GetAxialGeometry().MapSliceToWorldCoordinates(
                      static_cast<double>(x) * sx,
                      static_cast<double>(y) * sy) +
                    z * sz * g.GetAxialGeometry().GetNormal());
        ASSERT_TRUE(IsEqualVectorL1(p, q));
        
        q = (g.GetCoronalGeometry().MapSliceToWorldCoordinates(
               static_cast<double>(x) * sx,
               static_cast<double>(g.GetDepth() - 1 - z) * sz) +
             y * sy * g.GetCoronalGeometry().GetNormal());
        ASSERT_TRUE(IsEqualVectorL1(p, q));

        /**
         * WARNING: In sagittal geometry, the normal points to
         * REDUCING X-axis in the 3D world. This is necessary to keep
         * the right-hand coordinate system. Hence the "-".
         **/
        q = (g.GetSagittalGeometry().MapSliceToWorldCoordinates(
               static_cast<double>(y) * sy,
               static_cast<double>(g.GetDepth() - 1 - z) * sz) +
             x * sx * (-g.GetSagittalGeometry().GetNormal()));
        ASSERT_TRUE(IsEqualVectorL1(p, q));
      }
    }
  }

  ASSERT_EQ(0, (int) VolumeProjection_Axial);
  ASSERT_EQ(1, (int) VolumeProjection_Coronal);
  ASSERT_EQ(2, (int) VolumeProjection_Sagittal);
  
  for (int p = 0; p < 3; p++)
  {
    VolumeProjection projection = (VolumeProjection) p;
    const CoordinateSystem3D& s = g.GetProjectionGeometry(projection);
    
    ASSERT_THROW(g.GetProjectionSlice(projection, g.GetProjectionDepth(projection)), Orthanc::OrthancException);

    for (unsigned int i = 0; i < g.GetProjectionDepth(projection); i++)
    {
      CoordinateSystem3D plane = g.GetProjectionSlice(projection, i);

      if (projection == VolumeProjection_Sagittal)
      {
        ASSERT_TRUE(IsEqualVectorL1(plane.GetOrigin(), s.GetOrigin() + static_cast<double>(i) * 
                                    (-s.GetNormal()) * g.GetVoxelDimensions(projection)[2]));
      }
      else
      {
        ASSERT_TRUE(IsEqualVectorL1(plane.GetOrigin(), s.GetOrigin() + static_cast<double>(i) * 
                                    s.GetNormal() * g.GetVoxelDimensions(projection)[2]));
      }
      
      ASSERT_TRUE(IsEqualVectorL1(plane.GetAxisX(), s.GetAxisX()));
      ASSERT_TRUE(IsEqualVectorL1(plane.GetAxisY(), s.GetAxisY()));

      unsigned int slice;
      VolumeProjection q;
      ASSERT_TRUE(g.DetectSlice(q, slice, plane));
      ASSERT_EQ(projection, q);
      ASSERT_EQ(i, slice);
    }
  }
}


TEST(LinearAlgebra, ParseVectorLocale)
{
  OrthancStone::Vector v;

  ASSERT_TRUE(OrthancStone::LinearAlgebra::ParseVector(v, "1.2"));
  ASSERT_EQ(1u, v.size());
  ASSERT_DOUBLE_EQ(1.2, v[0]);

  ASSERT_TRUE(OrthancStone::LinearAlgebra::ParseVector(v, "-1.2e+2"));
  ASSERT_EQ(1u, v.size());
  ASSERT_DOUBLE_EQ(-120.0, v[0]);

  ASSERT_TRUE(OrthancStone::LinearAlgebra::ParseVector(v, "-1e-2\\2"));
  ASSERT_EQ(2u, v.size());
  ASSERT_DOUBLE_EQ(-0.01, v[0]);
  ASSERT_DOUBLE_EQ(2.0, v[1]);

  ASSERT_TRUE(OrthancStone::LinearAlgebra::ParseVector(v, "1.3671875\\1.3671875"));
  ASSERT_EQ(2u, v.size());
  ASSERT_DOUBLE_EQ(1.3671875, v[0]);
  ASSERT_DOUBLE_EQ(1.3671875, v[1]); 
}

TEST(GenericToolbox, NormalizeUuid)
{
  std::string ref("44ca5051-14ef-4d2f-8bd7-db20bfb61fbb");

  {
    std::string u("44ca5051-14ef-4d2f-8bd7-db20bfb61fbb");
    OrthancStone::GenericToolbox::NormalizeUuid(u);
    ASSERT_EQ(ref, u);
  }

  // space left
  {
    std::string u("  44ca5051-14ef-4d2f-8bd7-db20bfb61fbb");
    OrthancStone::GenericToolbox::NormalizeUuid(u);
    ASSERT_EQ(ref, u);
  }

  // space right
  {
    std::string u("44ca5051-14ef-4d2f-8bd7-db20bfb61fbb  ");
    OrthancStone::GenericToolbox::NormalizeUuid(u);
    ASSERT_EQ(ref, u);
  }

  // space l & r 
  {
    std::string u("  44ca5051-14ef-4d2f-8bd7-db20bfb61fbb  ");
    OrthancStone::GenericToolbox::NormalizeUuid(u);
    ASSERT_EQ(ref, u);
  }

  // space left + case 
  {
    std::string u("  44CA5051-14ef-4d2f-8bd7-dB20bfb61fbb");
    OrthancStone::GenericToolbox::NormalizeUuid(u);
    ASSERT_EQ(ref, u);
  }

  // space right + case 
  {
    std::string u("44ca5051-14EF-4D2f-8bd7-db20bfb61fbB  ");
    OrthancStone::GenericToolbox::NormalizeUuid(u);
    ASSERT_EQ(ref, u);
  }

  // space l & r + case
  {
    std::string u("  44cA5051-14Ef-4d2f-8bD7-db20bfb61fbb  ");
    OrthancStone::GenericToolbox::NormalizeUuid(u);
    ASSERT_EQ(ref, u);
  }

  // no
  {
    std::string u("  44ca5051-14ef-4d2f-8bd7-  db20bfb61fbb");
    OrthancStone::GenericToolbox::NormalizeUuid(u);
    ASSERT_NE(ref, u);
  }

  // no
  {
    std::string u("44ca5051-14ef-4d2f-8bd7-db20bfb61fb");
    OrthancStone::GenericToolbox::NormalizeUuid(u);
    ASSERT_NE(ref, u);
  }
}


TEST(CoordinateSystem3D, Basic)
{
  using namespace OrthancStone;
  
  {
    CoordinateSystem3D c;
    ASSERT_FALSE(c.IsValid());
    ASSERT_DOUBLE_EQ(c.GetNormal()[0], 0);
    ASSERT_DOUBLE_EQ(c.GetNormal()[1], 0);
    ASSERT_DOUBLE_EQ(c.GetNormal()[2], 1);

    ASSERT_DOUBLE_EQ(0, c.ComputeDistance(LinearAlgebra::CreateVector(0, 0, 0)));
    ASSERT_DOUBLE_EQ(0, c.ComputeDistance(LinearAlgebra::CreateVector(5, 0, 0)));
    ASSERT_DOUBLE_EQ(0, c.ComputeDistance(LinearAlgebra::CreateVector(0, 5, 0)));
    ASSERT_DOUBLE_EQ(5, c.ComputeDistance(LinearAlgebra::CreateVector(0, 0, 5)));
  }

  {
    CoordinateSystem3D c("nope1", "nope2");
    ASSERT_FALSE(c.IsValid());
    ASSERT_DOUBLE_EQ(c.GetNormal()[0], 0);
    ASSERT_DOUBLE_EQ(c.GetNormal()[1], 0);
    ASSERT_DOUBLE_EQ(c.GetNormal()[2], 1);
  }

  {
    // https://www.vedantu.com/maths/perpendicular-distance-of-a-point-from-a-plane
    CoordinateSystem3D c = CoordinateSystem3D::CreateFromPlaneGeneralForm(2, 4, -4, -6);
    ASSERT_DOUBLE_EQ(3, c.ComputeDistance(LinearAlgebra::CreateVector(0, 3, 6)));
  }

  {
    // https://mathinsight.org/distance_point_plane_examples
    CoordinateSystem3D c = CoordinateSystem3D::CreateFromPlaneGeneralForm(2, -2, 5, 8);
    ASSERT_DOUBLE_EQ(39.0 / sqrt(33.0), c.ComputeDistance(LinearAlgebra::CreateVector(4, -4, 3)));
  }

  {
    // https://www.ck12.org/calculus/distance-between-a-point-and-a-plane/lesson/Distance-Between-a-Point-and-a-Plane-MAT-ALY/
    const Vector a = LinearAlgebra::CreateVector(3, 6, 9);
    const Vector b = LinearAlgebra::CreateVector(9, 6, 3);
    const Vector c = LinearAlgebra::CreateVector(6, -9, 9);
    CoordinateSystem3D d = CoordinateSystem3D::CreateFromThreePoints(a, b, c);
    ASSERT_DOUBLE_EQ(0, d.ComputeDistance(a));
    ASSERT_DOUBLE_EQ(0, d.ComputeDistance(b));
    ASSERT_DOUBLE_EQ(0, d.ComputeDistance(c));
  }

  {
    // https://tutorial.math.lamar.edu/classes/calcii/eqnsofplanes.aspx
    const Vector a = LinearAlgebra::CreateVector(1, -2, 0);
    const Vector b = LinearAlgebra::CreateVector(3, 1, 4);
    const Vector c = LinearAlgebra::CreateVector(0, -1, 2);
    CoordinateSystem3D d = CoordinateSystem3D::CreateFromThreePoints(a, b, c);
    double r = d.GetNormal() [0] / 2.0;
    ASSERT_DOUBLE_EQ(-8 * r, d.GetNormal() [1]);
    ASSERT_DOUBLE_EQ(5 * r, d.GetNormal() [2]);
  }
}


TEST(SlicesSorter, HFP)
{
  // 2021-04-27-repro-bug-HFP-HFS-cartman
  
  {
    // This is the last instance in the CT series ("InstanceNumber" is 368):
    // CT1.2.752.243.1.1.20210202150623868.3730.61448.dcm
    const OrthancStone::CoordinateSystem3D system("300\\302.5\\323.11", "-1\\0\\0\\0\\-1\\0");

    /**
     * The first instance in the series ("InstanceNumber" is 1) is
     * CT1.2.752.243.1.1.20210202150623381.2000.76318.dcm, and its
     * "ImagePositionPatient" is "300\\302.5\\690.11". It cannot be
     * taken as the origin of the volume, otherwise the Z axis is
     * shifted by the depth of the volume.
     **/

    const double spacingXY = 1.171875;
    const double width = 512;
    const double height = 512;
    const double depth = 368;  // Number of instances in the series
  
    OrthancStone::VolumeImageGeometry geometry;
    geometry.SetAxialGeometry(system);
    geometry.SetSizeInVoxels(width, height, depth);
    geometry.SetVoxelDimensions(spacingXY, spacingXY, 1 /* pixel spacing Z */);

    OrthancStone::Vector p, q;

    p = OrthancStone::LinearAlgebra::CreateVector(0.5 / double(width),
                                                  0.5 / double(height),
                                                  0.5 / double(depth), 1);
    q = OrthancStone::LinearAlgebra::Product(geometry.GetTransform(), p);
    ASSERT_FLOAT_EQ(300, q[0]);
    ASSERT_FLOAT_EQ(302.5, q[1]);
    ASSERT_FLOAT_EQ(323.11, q[2]);
    ASSERT_FLOAT_EQ(1, q[3]);
  
    p = OrthancStone::LinearAlgebra::CreateVector((width - 0.5) / double(width),
                                                  (height - 0.5) / double(height),
                                                  (depth - 0.5) / double(depth), 1);
    q = OrthancStone::LinearAlgebra::Product(geometry.GetTransform(), p);

    ASSERT_FLOAT_EQ(300.0 - (width - 1.0) * spacingXY, q[0]);   // "X" is swapped
    ASSERT_FLOAT_EQ(302.5 - (height - 1.0) * spacingXY, q[1]);  // "Y" is swapped
    ASSERT_FLOAT_EQ(690.11, q[2]);
    ASSERT_FLOAT_EQ(1, q[3]);
  }

  {
    // DOSE instance: RD1.2.752.243.1.1.20210202150624529.3790.85357_DoseTPS.dcm
    OrthancStone::CoordinateSystem3D system("-217.0492\\-161.4141\\376.61", "1\\0\\0\\0\\1\\0");
    const double spacingXY = 3;
    const double width = 146; // Columns
    const double height = 84; // Row
    const double depth = 86;  // Number of frames, same as the length of "GridFrameOffsetVector"
  
    OrthancStone::VolumeImageGeometry geometry;
    geometry.SetAxialGeometry(system);
    geometry.SetSizeInVoxels(width, height, depth);
    geometry.SetVoxelDimensions(spacingXY, spacingXY, 3 /* pixel spacing Z, cf. "GridFrameOffsetVector" */);

    OrthancStone::Vector p, q;

    p = OrthancStone::LinearAlgebra::CreateVector(0.5 / double(width), 0.5 / double(height), 0.5 / double(depth), 1);
    q = OrthancStone::LinearAlgebra::Product(geometry.GetTransform(), p);
    ASSERT_FLOAT_EQ(-217.0492, q[0]);
    ASSERT_FLOAT_EQ(-161.4141, q[1]);
    ASSERT_FLOAT_EQ(376.61, q[2]);
    ASSERT_FLOAT_EQ(1, q[3]);
  
    p = OrthancStone::LinearAlgebra::CreateVector((width - 0.5) / double(width),
                                                  (height - 0.5) / double(height),
                                                  (depth - 0.5) / double(depth), 1);
    q = OrthancStone::LinearAlgebra::Product(geometry.GetTransform(), p);

    ASSERT_FLOAT_EQ(-217.0492 + (width - 1.0) * spacingXY, q[0]);
    ASSERT_FLOAT_EQ(-161.4141 + (height - 1.0) * spacingXY, q[1]);
    ASSERT_FLOAT_EQ(376.61 + 255.0 /* last item in "GridFrameOffsetVector" */, q[2]);
    ASSERT_FLOAT_EQ(1, q[3]);
  }

  for (unsigned int upward = 0; upward < 2; upward++)
  {
    OrthancStone::SlicesSorter slices;

    for (unsigned int i = 0; i < 368; i++)
    {
      unsigned int z = (upward ? 323 + i : 690 - i);
      OrthancStone::CoordinateSystem3D p("300\\302.5\\" + boost::lexical_cast<std::string>(z) + ".11",
                                         "-1\\0\\0\\0\\-1\\0");
      slices.AddSlice(p, new Orthanc::SingleValueObject<unsigned int>(z));
    }

    slices.Sort();

    double spacingZ;
    ASSERT_TRUE(slices.ComputeSpacingBetweenSlices(spacingZ));
    ASSERT_FLOAT_EQ(1, spacingZ);
    ASSERT_TRUE(slices.AreAllSlicesDistinct());

    ASSERT_FLOAT_EQ(300.0, slices.GetSliceGeometry(0).GetOrigin() [0]);
    ASSERT_FLOAT_EQ(302.5, slices.GetSliceGeometry(0).GetOrigin() [1]);
    ASSERT_FLOAT_EQ(323.11, slices.GetSliceGeometry(0).GetOrigin() [2]);

    ASSERT_FLOAT_EQ(300.0, slices.GetSliceGeometry(367).GetOrigin() [0]);
    ASSERT_FLOAT_EQ(302.5, slices.GetSliceGeometry(367).GetOrigin() [1]);
    ASSERT_FLOAT_EQ(690.11, slices.GetSliceGeometry(367).GetOrigin() [2]);

    ASSERT_EQ(323u, dynamic_cast<const Orthanc::SingleValueObject<unsigned int>&>(slices.GetSlicePayload(0)).GetValue());
    ASSERT_EQ(690u, dynamic_cast<const Orthanc::SingleValueObject<unsigned int>&>(slices.GetSlicePayload(367)).GetValue());

    const unsigned int width = 512;
    const unsigned int height = 512;
    const unsigned int depth = 368;
    const double spacingXY = 1.171875;
    
    OrthancStone::VolumeImageGeometry geometry;
    geometry.SetSizeInVoxels(width, height, depth);
    geometry.SetVoxelDimensions(spacingXY, spacingXY, 1);
    geometry.SetAxialGeometry(slices.GetSliceGeometry(0));

    OrthancStone::Vector q = geometry.GetCoordinates(0, 0, 0);
    ASSERT_EQ(3u, q.size());
    ASSERT_FLOAT_EQ(300 + spacingXY / 2.0, q[0]);
    ASSERT_FLOAT_EQ(302.5 + spacingXY / 2.0, q[1]);
    ASSERT_FLOAT_EQ(323.11 - 0.5, q[2]);

    q = geometry.GetCoordinates(1, 1, 1);
    ASSERT_EQ(3u, q.size());
    ASSERT_FLOAT_EQ(300 - double(width) * spacingXY + spacingXY / 2.0, q[0]);
    ASSERT_FLOAT_EQ(302.5 - double(height) * spacingXY + spacingXY / 2.0, q[1]);
    ASSERT_FLOAT_EQ(323.11 + 368.0 - 0.5, q[2]);

    OrthancStone::VolumeProjection projection;
    unsigned int slice;
    ASSERT_TRUE(geometry.DetectSlice(projection, slice, OrthancStone::CoordinateSystem3D("300\\302.5\\690.11", "-1\\0\\0\\0\\-1\\0")));
    ASSERT_EQ(OrthancStone::VolumeProjection_Axial, projection);
    ASSERT_EQ(367u, slice);
    ASSERT_TRUE(geometry.DetectSlice(projection, slice, OrthancStone::CoordinateSystem3D("300\\302.5\\323.11", "-1\\0\\0\\0\\-1\\0")));
    ASSERT_EQ(OrthancStone::VolumeProjection_Axial, projection);
    ASSERT_EQ(0u, slice);

    // DOSE instance: RD1.2.752.243.1.1.20210202150624529.3790.85357_DoseTPS.dcm
    ASSERT_TRUE(geometry.DetectSlice(projection, slice,
                                     OrthancStone::CoordinateSystem3D("-217.0492\\-161.4141\\376.61", "1\\0\\0\\0\\1\\0")));
    ASSERT_EQ(OrthancStone::VolumeProjection_Axial, projection);
    ASSERT_EQ(376u - 323u, slice);
  }
}
