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

#pragma once

#include "../Volumes/DicomVolumeImageMPRSlicer.h"
#include "OrthancSeriesVolumeProgressiveLoader.h"
#include "OrthancMultiframeVolumeLoader.h"
#include "DicomStructureSetLoader.h"

#include <boost/shared_ptr.hpp>

#include <map>
#include <string>
#include <vector>

namespace OrthancStone
{
  class ILoadersContext;
}

namespace OrthancStone
{
  class LoaderCache
  {
  public:

    virtual ~LoaderCache() {}

    /**
    By default, the CT loader in loader cache will only download the highest quality slices.
    If you pass true for useCtProgressiveQuality, jpeg (50/100 quality), then jpeg (90/100 quality) 
    then eventually uncompressed 16-bit images will be loaded. 
    */
    LoaderCache(OrthancStone::ILoadersContext& loadersContext,
                bool useCtProgressiveQuality);

    boost::shared_ptr<OrthancSeriesVolumeProgressiveLoader>
      GetSeriesVolumeProgressiveLoader      (std::string seriesUuid);
    
    boost::shared_ptr<OrthancStone::DicomVolumeImageMPRSlicer>
      GetMultiframeDicomVolumeImageMPRSlicer(std::string instanceUuid);

    boost::shared_ptr<OrthancMultiframeVolumeLoader>
      GetMultiframeVolumeLoader(std::string instanceUuid);

    /**
    The DicomStructureSetLoader instances are stored in a map and indexed
    by a key built from instanceUuid and uniqueKey.

    If instanceUuid and uniqueKey correspond to an already existing loader, it is returned.

    Please note that initiallyVisibleStructures is only used if the call results in the creation
    of a new loader. In that case, the value is passed to the constructor.
    */
    boost::shared_ptr<DicomStructureSetLoader>
      GetDicomStructureSetLoader(
        std::string instanceUuid,
        const std::vector<std::string>& initiallyVisibleStructures,
        const std::string& uniqueKey = "");

    std::string BuildDicomStructureSetLoaderKey(
        const std::string& instanceUuid,
        const std::string& uniqueKey = "");

    void ClearCache();

  protected:
    
    void DebugDisplayObjRefCounts();

    OrthancStone::ILoadersContext& loadersContext_;
    bool                           useCtProgressiveQuality_;

    std::map<std::string, boost::shared_ptr<OrthancSeriesVolumeProgressiveLoader> >
      seriesVolumeProgressiveLoaders_;
    std::map<std::string, boost::shared_ptr<OrthancMultiframeVolumeLoader> >
      multiframeVolumeLoaders_;
    std::map<std::string, boost::shared_ptr<OrthancStone::DicomVolumeImageMPRSlicer> >
      dicomVolumeImageMPRSlicers_;
    std::map<std::string, boost::shared_ptr<DicomStructureSetLoader> >
      dicomStructureSetLoaders_;
  };
}

