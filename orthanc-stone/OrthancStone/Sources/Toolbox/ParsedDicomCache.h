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

#include <Cache/MemoryObjectCache.h>
#include <DicomParsing/ParsedDicomFile.h>

namespace OrthancStone
{
  class ParsedDicomCache : public boost::noncopyable
  {
  private:
    class Item;

    static std::string GetIndex(unsigned int bucket,
                                const std::string& bucketKey);
    
    Orthanc::MemoryObjectCache  cache_;
    size_t                      lowCacheSizeWarning_;

  public:
    explicit ParsedDicomCache(size_t size) :
      lowCacheSizeWarning_(0)
    {
      cache_.SetMaximumSize(size);
    }

    void Invalidate(unsigned int bucket,
                    const std::string& bucketKey)
    {
      cache_.Invalidate(GetIndex(bucket, bucketKey));
    }
    
    void Acquire(unsigned int bucket,
                 const std::string& bucketKey,
                 Orthanc::ParsedDicomFile* dicom,
                 size_t fileSize,
                 bool hasPixelData);

    class Reader : public boost::noncopyable
    {
    private:
      Orthanc::MemoryObjectCache::Accessor accessor_;
      Item*                                item_;

    public:
      Reader(ParsedDicomCache& cache,
             unsigned int bucket,
             const std::string& bucketKey);

      bool IsValid() const
      {
        return item_ != NULL;
      }

      bool HasPixelData() const;

      Orthanc::ParsedDicomFile& GetDicom() const;

      size_t GetFileSize() const;
    };
  };
}
