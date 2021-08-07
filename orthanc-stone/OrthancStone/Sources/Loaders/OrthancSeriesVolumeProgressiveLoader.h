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

#include "../Loaders/IFetchingItemsSorter.h"
#include "../Loaders/IFetchingStrategy.h"
#include "../Messages/IObservable.h"
#include "../Messages/ObserverBase.h"
#include "../Oracle/GetOrthancImageCommand.h"
#include "../Oracle/GetOrthancWebViewerJpegCommand.h"
#include "../Oracle/IOracle.h"
#include "../Oracle/OrthancRestApiCommand.h"
#include "../Toolbox/SlicesSorter.h"
#include "../Volumes/DicomVolumeImage.h"
#include "../Volumes/IVolumeSlicer.h"

#include "../Volumes/IGeometryProvider.h"


#include <boost/shared_ptr.hpp>

namespace OrthancStone
{
  class ILoadersContext;
  /**
    This class is used to manage the progressive loading of a volume that
    is stored in a Dicom series.
  */
  class OrthancSeriesVolumeProgressiveLoader : 
    public ObserverBase<OrthancSeriesVolumeProgressiveLoader>,
    public IObservable,
    public IVolumeSlicer,
    public IGeometryProvider
  {
  public:
    class ISlicePostProcessor
    {
    public:
      virtual void ProcessCTDicomSlice(const Orthanc::DicomMap& dicom) = 0;
    };

  private:
    static const unsigned int QUALITY_00 = 0;
    static const unsigned int QUALITY_01 = 1;
    static const unsigned int QUALITY_02 = 2;
        
    class ExtractedSlice;

   
    /** Helper class internal to OrthancSeriesVolumeProgressiveLoader */
    class SeriesGeometry : public boost::noncopyable
    {
    private:
      void CheckSlice(size_t index,
                      const DicomInstanceParameters& reference) const;
    
      void CheckVolume() const;

      void Clear();

      void CheckSliceIndex(size_t index) const;

      std::unique_ptr<VolumeImageGeometry>   geometry_;
      std::vector<DicomInstanceParameters*>  slices_;
      std::vector<uint64_t>                  slicesRevision_;

    public:
      ~SeriesGeometry()
      {
        Clear();
      }

      void ComputeGeometry(SlicesSorter& slices);

      virtual bool HasGeometry() const
      {
        return geometry_.get() != NULL;
      }

      virtual const VolumeImageGeometry& GetImageGeometry() const;

      const DicomInstanceParameters& GetSliceParameters(size_t index) const;

      uint64_t GetSliceRevision(size_t index) const;

      void IncrementSliceRevision(size_t index);
    };

    void ScheduleNextSliceDownload();

    void LoadGeometry(const OrthancRestApiCommand::SuccessMessage& message);

    void SetSliceContent(unsigned int sliceIndex,
                         const Orthanc::ImageAccessor& image,
                         unsigned int quality);

    void LoadBestQualitySliceContent(const GetOrthancImageCommand::SuccessMessage& message);

    void LoadJpegSliceContent(const GetOrthancWebViewerJpegCommand::SuccessMessage& message);

    ILoadersContext&                                loadersContext_;
    bool                                            active_;
    bool                                            progressiveQuality_;
    bool                                            startCenter_;
    unsigned int                                    simultaneousDownloads_;
    SeriesGeometry                                  seriesGeometry_;
    boost::shared_ptr<DicomVolumeImage>             volume_;
    std::unique_ptr<IFetchingItemsSorter::IFactory> sorter_;
    std::unique_ptr<IFetchingStrategy>              strategy_;
    std::vector<unsigned int>                       slicesQuality_;
    bool                                            volumeImageReadyInHighQuality_;
    boost::shared_ptr<ISlicePostProcessor>          slicePostProcessor_;

    /** See priority setters/getters below */
    int medadataSchedulingPriority_;

    /** See priority setters/getters below */
    int sliceSchedulingPriority_;

    OrthancSeriesVolumeProgressiveLoader(
      ILoadersContext& loadersContext,
      boost::shared_ptr<DicomVolumeImage> volume,
      bool progressiveQuality);
  
  public:
    ORTHANC_STONE_DEFINE_ORIGIN_MESSAGE(__FILE__, __LINE__, VolumeImageReadyInHighQuality, OrthancSeriesVolumeProgressiveLoader);

    /**
    See doc for the progressiveQuality_ field
    */
    static boost::shared_ptr<OrthancSeriesVolumeProgressiveLoader> Create(
      ILoadersContext& context,
      boost::shared_ptr<DicomVolumeImage> volume,
      bool progressiveQuality = false);

    virtual ~OrthancSeriesVolumeProgressiveLoader();

    void SetStartCenter(bool startCenter);

    void SetSimultaneousDownloads(unsigned int count);

    /**
      Sets the relative priority of the requests for metadata.
      - if p < PRIORITY_HIGH (-1)                 , the requests will be high priority
      - if PRIORITY_LOW (100) > p > PRIORITY_HIGH , the requests will be medium priority
      - if p > PRIORITY_LOW                       , the requests will be low priority

      Default is 0 (medium)
    */
    void  SetMetadataSchedulingPriority(int p);

    /** @see SetMetadataSchedulingPriority */
    int   GetMetadataSchedulingPriority() const;

    /** Same as SetMetadataSchedulingPriority, for slices. Default is 0. */
    void  SetSliceSchedulingPriority(int p);
    
    /** @see SetSliceSchedulingPriority */
    int   GetSliceSchedulingPriority() const;

    /** Sets priorities for all requests. @see SetMetadataSchedulingPriority */
    void  SetSchedulingPriority(int p);

    void SetDicomSlicePostProcessor(boost::shared_ptr<ISlicePostProcessor> slicePostProcessor)
    {
      // this will delete the previously stored slice processor, if any
      slicePostProcessor_ = slicePostProcessor;
    }

    boost::shared_ptr<ISlicePostProcessor> GetDicomSlicePostProcessor()
    {
      // this could be empty!
      return slicePostProcessor_;
    }

    bool IsVolumeImageReadyInHighQuality() const
    {
      return volumeImageReadyInHighQuality_;
    }

    void LoadSeries(const std::string& seriesId);

    /**
    This getter is used by clients that do not receive the geometry through
    subscribing, for instance if they are created or listening only AFTER the
    "geometry loaded" message is broadcast 
    */
    bool HasGeometry() const ORTHANC_OVERRIDE
    {
      return seriesGeometry_.HasGeometry();
    }

    /**
    Same remark as HasGeometry
    */
    const VolumeImageGeometry& GetImageGeometry() const ORTHANC_OVERRIDE
    {
      return seriesGeometry_.GetImageGeometry();
    }

    /**
    When a slice is requested, the strategy algorithm (that defines the 
    sequence of resources to be loaded from the server) is modified to 
    take into account this request (this is done in the ExtractedSlice ctor)
    */
    virtual IExtractedSlice*
      ExtractSlice(const CoordinateSystem3D& cuttingPlane) ORTHANC_OVERRIDE;
  };
}
