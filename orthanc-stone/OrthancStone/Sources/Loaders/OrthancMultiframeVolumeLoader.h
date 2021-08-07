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

#include "LoaderStateMachine.h"
#include "../Volumes/DicomVolumeImage.h"
#include "../Volumes/IGeometryProvider.h"

#include <boost/shared_ptr.hpp>

namespace OrthancStone
{
  class OrthancMultiframeVolumeLoader :
    public LoaderStateMachine,
    public IObservable,
    public IGeometryProvider
  {
  private:
    class LoadRTDoseGeometry;
    class LoadGeometry;
    class LoadTransferSyntax;    
    class LoadUncompressedPixelData;

    struct PixelCount
    {
        uint64_t count_;
        PixelCount() { count_ = 0; }
    };

    boost::shared_ptr<DicomVolumeImage>  volume_;
    std::string                          instanceId_;
    std::string                          transferSyntaxUid_;
    bool                                 pixelDataLoaded_;
    float                                outliersHalfRejectionRate_;
    float                                distributionRawMin_;
    float                                distributionRawMax_;
    float                                computedDistributionMin_;
    float                                computedDistributionMax_;

    const std::string& GetInstanceId() const;

    void ScheduleFrameDownloads();

    void SetTransferSyntax(const std::string& transferSyntax);

    void SetGeometry(const Orthanc::DicomMap& dicom);


    /**
    This method will :
    
    - copy the pixel values from the response to the volume image
    - compute the maximum and minimum value while discarding the
      outliersHalfRejectionRate_ fraction of the outliers from both the start 
      and the end of the distribution.

      In English, this means that, if the volume dataset contains a few extreme
      values very different from the rest (outliers) that we want to get rid of,
      this method allows to do so.

      If you supply 0.005, for instance, it means 1% of the extreme values will
      be rejected (0.5% on each side of the distribution)
    */
    template <typename T>
    void CopyPixelDataAndComputeMinMax(const std::string& pixelData);
      
    /** Service method for CopyPixelDataAndComputeMinMax*/
    template <typename T>
    void CopyPixelDataAndComputeDistribution(const std::string& pixelData, 
                                             std::map<T, PixelCount>& distribution);

    /** Service method for CopyPixelDataAndComputeMinMax*/
    template <typename T>
    void ComputeMinMaxWithOutlierRejection(const std::map<T, PixelCount>& distribution);

    void SetUncompressedPixelData(const std::string& pixelData);

  protected:
    OrthancMultiframeVolumeLoader(ILoadersContext& loadersContext,
                                  boost::shared_ptr<DicomVolumeImage> volume,
                                  float outliersHalfRejectionRate);
    
  public:
    static boost::shared_ptr<OrthancMultiframeVolumeLoader> Create(
      ILoadersContext& loadersContext,
      boost::shared_ptr<DicomVolumeImage> volume,
      float outliersHalfRejectionRate = 0.0005);

    virtual ~OrthancMultiframeVolumeLoader();

    bool HasGeometry() const ORTHANC_OVERRIDE;
    
    virtual const VolumeImageGeometry& GetImageGeometry() const ORTHANC_OVERRIDE;

    bool IsPixelDataLoaded() const
    {
      return pixelDataLoaded_;
    }

    void GetDistributionMinMax
      (float& minValue, float& maxValue) const;

    void GetDistributionMinMaxWithOutliersRejection
      (float& minValue, float& maxValue) const;

    void LoadInstance(const std::string& instanceId);
  };
}
