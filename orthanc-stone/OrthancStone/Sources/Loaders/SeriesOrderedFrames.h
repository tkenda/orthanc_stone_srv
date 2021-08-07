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

#include "LoadedDicomResources.h"

#include "../Toolbox/DicomInstanceParameters.h"

namespace OrthancStone
{
  class SeriesOrderedFrames : public boost::noncopyable
  {
  private:
    class Instance;
    class Frame;
    class InstanceWithIndexInSeries;

    std::vector<Instance*>  instances_;
    std::vector<Frame*>     orderedFrames_;
    bool                    isVolume_;
    bool                    isRegular_;
    double                  spacingBetweenSlices_;

    void Clear();

    bool Sort3DVolume();

    void SortIndexInSeries();

    const Frame& GetFrame(size_t seriesIndex) const;

  public:
    explicit SeriesOrderedFrames(LoadedDicomResources& instances);

    ~SeriesOrderedFrames()
    {
      Clear();
    }

    size_t GetFramesCount() const
    {
      return orderedFrames_.size();
    }

    unsigned int GetFrameIndex(size_t seriesIndex) const;

    const Orthanc::DicomMap& GetInstance(size_t seriesIndex) const;

    const DicomInstanceParameters& GetInstanceParameters(size_t seriesIndex) const;

    // Are all frames parallel and aligned?
    bool Is3DVolume() const
    {
      return isVolume_;
    }

    // Are all frames parallel, aligned and evenly spaced?
    bool IsRegular3DVolume() const
    {
      return isRegular_;
    }

    // Only available on regular 3D volumes
    double GetSpacingBetweenSlices() const;
  };
}
