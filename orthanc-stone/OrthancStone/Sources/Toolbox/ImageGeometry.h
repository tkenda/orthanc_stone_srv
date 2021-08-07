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

#include "../StoneEnumerations.h"
#include "LinearAlgebra.h"

#include <Images/ImageAccessor.h>


namespace OrthancStone
{
  // Returns the "useful" portion of the target image when applying a
  // 3x3 perspective transform "a" (i.e. the bounding box where points
  // of the source image are mapped to)
  bool GetProjectiveTransformExtent(unsigned int& x1,
                                    unsigned int& y1,
                                    unsigned int& x2,
                                    unsigned int& y2,
                                    const Matrix& a,
                                    unsigned int sourceWidth,
                                    unsigned int sourceHeight,
                                    unsigned int targetWidth,
                                    unsigned int targetHeight);

  void ApplyAffineTransform(Orthanc::ImageAccessor& target,
                            const Orthanc::ImageAccessor& source,
                            double a11,
                            double a12,
                            double b1,
                            double a21,
                            double a22,
                            double b2,
                            ImageInterpolation interpolation,
                            bool clear);

  void ApplyProjectiveTransform(Orthanc::ImageAccessor& target,
                                const Orthanc::ImageAccessor& source,
                                const Matrix& a,
                                ImageInterpolation interpolation,
                                bool clear);
}
