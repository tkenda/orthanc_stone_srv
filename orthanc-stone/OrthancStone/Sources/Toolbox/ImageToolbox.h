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

  /**
  This structure represents the result of an histogram computation

  bins[0]   contains the values in [minValue                , minValue +   binSize [
  bins[1]   contains the values in [minValue +       binSize, minValue + 2*binSize [
  bins[2]   contains the values in [minValue +     2*binSize, minValue + 3*binSize [
  ...
  bins[N-1] contains the values in [minValue + (N-1)*binSize, minValue + N*binSize [

  */
  struct HistogramData
  {
    std::vector<size_t> bins;
    double minValue;
    double binSize;
  };

  /**
  Dumps the supplied histogram to the supplied strings
  */
  void DumpHistogramResult(std::string& s, const HistogramData& hd);
  
  /**
  This will compute the histogram of the supplied image (count the number of 
  pixels).

  The image must contain arithmetic pixels (that is, having a single component,
  integer or float). Compound pixel types like RGB, YUV are not supported and
  will cause this function to throw an exception.

  The range of available values will be split in sets of size `binSize`, and 
  each set will contain the number of pixels in the given bin 
  (see HistogramResult above).
  */
  void ComputeHistogram(const Orthanc::ImageAccessor& img,
                        HistogramData& hd, double binSize);


  /**
  Computes the min max values in an image
  */
  void ComputeMinMax(const Orthanc::ImageAccessor& img, 
                     double& minValue, double& maxValue);


  class ImageToolbox
  {
  public:
    static bool IsDecodingSupported(Orthanc::DicomTransferSyntax& transferSyntax);
  };
}
