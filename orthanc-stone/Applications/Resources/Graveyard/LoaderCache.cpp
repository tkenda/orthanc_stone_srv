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

#include "LoaderCache.h"

#include "../StoneException.h"
#include "OrthancSeriesVolumeProgressiveLoader.h"
#include "OrthancMultiframeVolumeLoader.h"
#include "DicomStructureSetLoader.h"
#include "../Toolbox/GenericToolbox.h"

#include "../Loaders/ILoadersContext.h"

#if ORTHANC_ENABLE_WASM == 1
# include <unistd.h>
# include "../Oracle/WebAssemblyOracle.h"
#else
# include "../Oracle/ThreadedOracle.h"
#endif

#include "../Volumes/DicomVolumeImage.h"
#include "../Volumes/DicomVolumeImageMPRSlicer.h"

#include <OrthancException.h>
#include <Toolbox.h>

namespace OrthancStone
{
  LoaderCache::LoaderCache(OrthancStone::ILoadersContext& loadersContext, bool useCtProgressiveQuality)
    : loadersContext_(loadersContext)
    , useCtProgressiveQuality_(useCtProgressiveQuality)

  {

  }

  boost::shared_ptr<OrthancSeriesVolumeProgressiveLoader> 
    LoaderCache::GetSeriesVolumeProgressiveLoader(std::string seriesUuid)
  {
    try
    {
      // normalize keys a little
      GenericToolbox::NormalizeUuid(seriesUuid);

      // find in cache
      if (seriesVolumeProgressiveLoaders_.find(seriesUuid) == seriesVolumeProgressiveLoaders_.end())
      {
        std::unique_ptr<OrthancStone::ILoadersContext::ILock> lock(loadersContext_.Lock());

        boost::shared_ptr<OrthancStone::DicomVolumeImage> volumeImage(new OrthancStone::DicomVolumeImage);
        boost::shared_ptr<OrthancSeriesVolumeProgressiveLoader> loader;
      
        // true means "use progressive quality"
        // false means "load high quality slices only"
        loader = OrthancSeriesVolumeProgressiveLoader::Create(loadersContext_, volumeImage, useCtProgressiveQuality_);
        loader->LoadSeries(seriesUuid);
        seriesVolumeProgressiveLoaders_[seriesUuid] = loader;
      }
      else
      {
//        LOG(TRACE) << "LoaderCache::GetSeriesVolumeProgressiveLoader : returning cached loader for seriesUUid = " << seriesUuid;
      }
      return seriesVolumeProgressiveLoaders_[seriesUuid];
    }
    catch (const Orthanc::OrthancException& e)
    {
      if (e.HasDetails())
      {
        LOG(ERROR) << "OrthancException in LoaderCache: " << e.What() << " Details: " << e.GetDetails();
      }
      else
      {
        LOG(ERROR) << "OrthancException in LoaderCache: " << e.What();
      }
      throw;
    }
    catch (const std::exception& e)
    {
      LOG(ERROR) << "std::exception in LoaderCache: " << e.what();
      throw;
    }
    catch (...)
    {
      LOG(ERROR) << "Unknown exception in LoaderCache";
      throw;
    }
  }

  boost::shared_ptr<OrthancMultiframeVolumeLoader> LoaderCache::GetMultiframeVolumeLoader(std::string instanceUuid)
  {
    // normalize keys a little
    GenericToolbox::NormalizeUuid(instanceUuid);

    // if the loader is not available, let's trigger its creation
    if(multiframeVolumeLoaders_.find(instanceUuid) == multiframeVolumeLoaders_.end())
    {
      GetMultiframeDicomVolumeImageMPRSlicer(instanceUuid);
    }
    ORTHANC_ASSERT(multiframeVolumeLoaders_.find(instanceUuid) != multiframeVolumeLoaders_.end());

    return multiframeVolumeLoaders_[instanceUuid];
  }

  boost::shared_ptr<OrthancStone::DicomVolumeImageMPRSlicer> LoaderCache::GetMultiframeDicomVolumeImageMPRSlicer(std::string instanceUuid)
  {
    try
    {
      // normalize keys a little
      GenericToolbox::NormalizeUuid(instanceUuid);

      // find in cache
      if (dicomVolumeImageMPRSlicers_.find(instanceUuid) == dicomVolumeImageMPRSlicers_.end())
      {
        std::unique_ptr<OrthancStone::ILoadersContext::ILock> lock(loadersContext_.Lock());
        boost::shared_ptr<OrthancStone::DicomVolumeImage> volumeImage(new OrthancStone::DicomVolumeImage);
        boost::shared_ptr<OrthancMultiframeVolumeLoader> loader;
        {
          loader = OrthancMultiframeVolumeLoader::Create(loadersContext_, volumeImage);
          loader->LoadInstance(instanceUuid);
        }
        multiframeVolumeLoaders_[instanceUuid] = loader;
        boost::shared_ptr<OrthancStone::DicomVolumeImageMPRSlicer> mprSlicer(new OrthancStone::DicomVolumeImageMPRSlicer(volumeImage));
        dicomVolumeImageMPRSlicers_[instanceUuid] = mprSlicer;
      }
      return dicomVolumeImageMPRSlicers_[instanceUuid];
    }
    catch (const Orthanc::OrthancException& e)
    {
      if (e.HasDetails())
      {
        LOG(ERROR) << "OrthancException in LoaderCache: " << e.What() << " Details: " << e.GetDetails();
      }
      else
      {
        LOG(ERROR) << "OrthancException in LoaderCache: " << e.What();
      }
      throw;
    }
    catch (const std::exception& e)
    {
      LOG(ERROR) << "std::exception in LoaderCache: " << e.what();
      throw;
    }
    catch (...)
    {
      LOG(ERROR) << "Unknown exception in LoaderCache";
      throw;
    }
  }
  
  std::string LoaderCache::BuildDicomStructureSetLoaderKey(
    const std::string& instanceUuid,
    const std::string& uniqueKey)
  {
    return instanceUuid + "_" + uniqueKey;
  }

  boost::shared_ptr<DicomStructureSetLoader> LoaderCache::GetDicomStructureSetLoader(
      std::string inInstanceUuid, 
      const std::vector<std::string>& initiallyVisibleStructures,
      const std::string& uniqueKey)
  {
    try
    {
      // normalize keys a little
      GenericToolbox::NormalizeUuid(inInstanceUuid);

      std::string entryKey = BuildDicomStructureSetLoaderKey(inInstanceUuid, uniqueKey);

      // find in cache
      if (dicomStructureSetLoaders_.find(entryKey) == dicomStructureSetLoaders_.end())
      {
        std::unique_ptr<OrthancStone::ILoadersContext::ILock> lock(loadersContext_.Lock());

        boost::shared_ptr<DicomStructureSetLoader> loader;
        {
          loader = DicomStructureSetLoader::Create(loadersContext_);
          loader->LoadInstance(inInstanceUuid, initiallyVisibleStructures);
        }
        dicomStructureSetLoaders_[entryKey] = loader;
      }
      return dicomStructureSetLoaders_[entryKey];
    }
    catch (const Orthanc::OrthancException& e)
    {
      if (e.HasDetails())
      {
        LOG(ERROR) << "OrthancException in LoaderCache: " << e.What() << " Details: " << e.GetDetails();
      }
      else
      {
        LOG(ERROR) << "OrthancException in LoaderCache: " << e.What();
      }
      throw;
    }
    catch (const std::exception& e)
    {
      LOG(ERROR) << "std::exception in LoaderCache: " << e.what();
      throw;
    }
    catch (...)
    {
      LOG(ERROR) << "Unknown exception in LoaderCache";
      throw;
    }
  }

  void LoaderCache::ClearCache()
  {
    std::unique_ptr<OrthancStone::ILoadersContext::ILock> lock(loadersContext_.Lock());
    
#ifndef NDEBUG
    // ISO way of checking for debug builds
    DebugDisplayObjRefCounts();
#endif
    seriesVolumeProgressiveLoaders_.clear();
    multiframeVolumeLoaders_.clear();
    dicomVolumeImageMPRSlicers_.clear();
    dicomStructureSetLoaders_.clear();

  }

  template<typename T> void DebugDisplayObjRefCountsInMap(
    const std::string& name, const std::map<std::string, boost::shared_ptr<T> >& myMap)
  {
    LOG(TRACE) << "Map \"" << name << "\" ref counts:";
    size_t i = 0;
    for (typename std::map<std::string, boost::shared_ptr<T> >::const_iterator 
           it = myMap.begin(); it != myMap.end(); ++it)
    {
      LOG(TRACE) << "  element #" << i << ": ref count = " << it->second.use_count();
      i++;
    }
  }

  void LoaderCache::DebugDisplayObjRefCounts()
  {
    DebugDisplayObjRefCountsInMap("seriesVolumeProgressiveLoaders_", seriesVolumeProgressiveLoaders_);
    DebugDisplayObjRefCountsInMap("multiframeVolumeLoaders_", multiframeVolumeLoaders_);
    DebugDisplayObjRefCountsInMap("dicomVolumeImageMPRSlicers_", dicomVolumeImageMPRSlicers_);
    DebugDisplayObjRefCountsInMap("dicomStructureSetLoaders_", dicomStructureSetLoaders_);
  }
}
