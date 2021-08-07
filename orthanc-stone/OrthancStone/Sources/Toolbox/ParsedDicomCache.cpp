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


#include "ParsedDicomCache.h"

#include <Logging.h>

#include <cassert>


namespace OrthancStone
{
  class ParsedDicomCache::Item : public Orthanc::ICacheable
  {
  private:
    std::unique_ptr<Orthanc::ParsedDicomFile>  dicom_;
    size_t                                     fileSize_;
    bool                                       hasPixelData_;
    
  public:
    Item(Orthanc::ParsedDicomFile* dicom,
         size_t fileSize,
         bool hasPixelData) :
      dicom_(dicom),
      fileSize_(fileSize),
      hasPixelData_(hasPixelData)
    {
      if (dicom == NULL)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
      }
    }
           
    virtual size_t GetMemoryUsage() const ORTHANC_OVERRIDE
    {
      return fileSize_;
    }

    Orthanc::ParsedDicomFile& GetDicom() const
    {
      assert(dicom_.get() != NULL);
      return *dicom_;
    }

    bool HasPixelData() const
    {
      return hasPixelData_;
    }
  };
    

  std::string ParsedDicomCache::GetIndex(unsigned int bucket,
                                         const std::string& bucketKey)
  {
    return boost::lexical_cast<std::string>(bucket) + "|" + bucketKey;
  }
  

  void ParsedDicomCache::Acquire(unsigned int bucket,
                                 const std::string& bucketKey,
                                 Orthanc::ParsedDicomFile* dicom,
                                 size_t fileSize,
                                 bool hasPixelData)
  {
    LOG(TRACE) << "new item stored in cache: bucket " << bucket << ", key " << bucketKey;

    if (lowCacheSizeWarning_ < fileSize &&
        cache_.GetMaximumSize() > 0 &&
        fileSize >= cache_.GetMaximumSize())
    {
      lowCacheSizeWarning_ = fileSize;
      LOG(WARNING) << "The DICOM cache size should be larger: Storing a DICOM instance of "
                   << (fileSize / (1024 * 1024)) << "MB, whereas the cache size is only "
                   << (cache_.GetMaximumSize() / (1024 * 1024)) << "MB wide";
    }
    
    cache_.Acquire(GetIndex(bucket, bucketKey), new Item(dicom, fileSize, hasPixelData));
  }

  
  ParsedDicomCache::Reader::Reader(ParsedDicomCache& cache,
                                   unsigned int bucket,
                                   const std::string& bucketKey) :
    /**
     * The "DcmFileFormat" object cannot be accessed from multiple
     * threads, even if using only getters. An unique lock (mutex) is
     * mandatory.
     **/
    accessor_(cache.cache_, GetIndex(bucket, bucketKey), true /* unique */)
  {
    if (accessor_.IsValid())
    {
      LOG(TRACE) << "accessing item within cache: bucket " << bucket << ", key " << bucketKey;
      item_ = &dynamic_cast<Item&>(accessor_.GetValue());
    }
    else
    {
      LOG(TRACE) << "missing item within cache: bucket " << bucket << ", key " << bucketKey;
      item_ = NULL;
    }
  }


  bool ParsedDicomCache::Reader::HasPixelData() const
  {
    if (item_ == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    else
    {
      return item_->HasPixelData();
    }
  }

  
  Orthanc::ParsedDicomFile& ParsedDicomCache::Reader::GetDicom() const
  {
    if (item_ == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    else
    {
      return item_->GetDicom();
    }
  }

  
  size_t ParsedDicomCache::Reader::GetFileSize() const
  {
    if (item_ == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    else
    {
      return item_->GetMemoryUsage();
    }
  }
}
