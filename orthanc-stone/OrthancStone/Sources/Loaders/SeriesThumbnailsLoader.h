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

#include "../OrthancStone.h"

#if !defined(ORTHANC_ENABLE_DCMTK)
#  error Macro ORTHANC_ENABLE_DCMTK must be defined
#endif 


#include "../Oracle/GetOrthancImageCommand.h"
#include "../Oracle/HttpCommand.h"
#include "../Oracle/OracleCommandExceptionMessage.h"
#include "../Oracle/OrthancRestApiCommand.h"
#include "DicomSource.h"
#include "ILoaderFactory.h"
#include "OracleScheduler.h"


namespace OrthancStone
{
  class SeriesThumbnailsLoader :
    public IObservable,
    public ObserverBase<SeriesThumbnailsLoader>
  {
  private:
    class Thumbnail : public boost::noncopyable
    {
    private:
      SeriesThumbnailType  type_;
      std::string          image_;
      std::string          mime_;

    public:
      Thumbnail(const std::string& image,
                const std::string& mime);

      explicit Thumbnail(SeriesThumbnailType type);

      SeriesThumbnailType GetType() const
      {
        return type_;
      }

      const std::string& GetImage() const
      {
        return image_;
      }

      const std::string& GetMime() const
      {
        return mime_;
      }
    };

  public:
    class SuccessMessage : public OriginMessage<SeriesThumbnailsLoader>
    {
      ORTHANC_STONE_MESSAGE(__FILE__, __LINE__);
      
    private:
      const DicomSource&   source_;
      const std::string&   studyInstanceUid_;
      const std::string&   seriesInstanceUid_;
      const Thumbnail&     thumbnail_;

    public:
      SuccessMessage(const SeriesThumbnailsLoader& origin,
                     const DicomSource& source,
                     const std::string& studyInstanceUid,
                     const std::string& seriesInstanceUid,
                     const Thumbnail& thumbnail) :
        OriginMessage(origin),
        source_(source),
        studyInstanceUid_(studyInstanceUid),
        seriesInstanceUid_(seriesInstanceUid),
        thumbnail_(thumbnail)
      {
      }

      const DicomSource& GetDicomSource() const
      {
        return source_;
      }

      SeriesThumbnailType GetType() const
      {
        return thumbnail_.GetType();
      }

      const std::string& GetStudyInstanceUid() const
      {
        return studyInstanceUid_;
      }

      const std::string& GetSeriesInstanceUid() const
      {
        return seriesInstanceUid_;
      }

      const std::string& GetEncodedImage() const
      {
        return thumbnail_.GetImage();
      }

      const std::string& GetMime() const
      {
        return thumbnail_.GetMime();
      }

      Orthanc::ImageAccessor* DecodeImage() const;
    };

  private:
    class Handler;
    class DicomWebSopClassHandler;
    class DicomWebThumbnailHandler;
    class ThumbnailInformation;
    class OrthancSopClassHandler;
    class SelectOrthancInstanceHandler;

#if ORTHANC_ENABLE_DCMTK == 1
    class SelectDicomWebInstanceHandler;
#endif
    
    // Maps a "Series Instance UID" to a thumbnail
    typedef std::map<std::string, Thumbnail*>  Thumbnails;

    ILoadersContext&       context_;
    Thumbnails             thumbnails_;
    int                    priority_;
    unsigned int           width_;
    unsigned int           height_;
    std::set<std::string>  scheduledSeries_;

    void AcquireThumbnail(const DicomSource& source,
                          const std::string& studyInstanceUid,
                          const std::string& seriesInstanceUid,
                          Thumbnail* thumbnail /* takes ownership */);

    void Schedule(IOracleCommand* command);
  
    void Handle(const HttpCommand::SuccessMessage& message);

    void Handle(const OrthancRestApiCommand::SuccessMessage& message);

    void Handle(const GetOrthancImageCommand::SuccessMessage& message);

#if ORTHANC_ENABLE_DCMTK == 1
    void Handle(const ParseDicomSuccessMessage& message);
#endif

    void Handle(const OracleCommandExceptionMessage& message);

    SeriesThumbnailsLoader(ILoadersContext& context,
                           int priority);
    
  public:
    class Factory : public ILoaderFactory
    {
    private:
      int priority_;

    public:
      Factory() :
        priority_(0)
      {
      }

      void SetPriority(int priority)
      {
        priority_ = priority;
      }

      virtual boost::shared_ptr<IObserver> Create(ILoadersContext::ILock& context)
      {
        return SeriesThumbnailsLoader::Create(context, priority_);
      }
    };


    virtual ~SeriesThumbnailsLoader()
    {
      Clear();
    }


    static boost::shared_ptr<SeriesThumbnailsLoader> Create(const ILoadersContext::ILock& context,
                                                            int priority);

    void SetThumbnailSize(unsigned int width,
                          unsigned int height);
    
    void Clear();

    unsigned int GetThumbnailWidth() const
    {
      return width_;      
    }

    unsigned int GetThumbnailHeight() const
    {
      return height_;      
    }
    
    SeriesThumbnailType GetSeriesThumbnail(std::string& image,
                                           std::string& mime,
                                           const std::string& seriesInstanceUid) const;

    void ScheduleLoadThumbnail(const DicomSource& source,
                               const std::string& patientId,
                               const std::string& studyInstanceUid,
                               const std::string& seriesInstanceUid);

    bool IsScheduledSeries(const std::string& seriesInstanceUid) const
    {
      return scheduledSeries_.find(seriesInstanceUid) != scheduledSeries_.end();
    }
  };
}
