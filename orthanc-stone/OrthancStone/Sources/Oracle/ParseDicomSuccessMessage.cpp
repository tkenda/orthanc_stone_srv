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


#include "ParseDicomSuccessMessage.h"

#include <DicomParsing/ParsedDicomFile.h>
#include <HttpServer/MultipartStreamReader.h>
#include <OrthancException.h>

namespace OrthancStone
{
  class MultipartHandler : public Orthanc::MultipartStreamReader::IHandler
  {
  private:
    std::unique_ptr<Orthanc::ParsedDicomFile>  dicom_;
    size_t                                   size_;

  public:
    MultipartHandler() :
      size_(0)
    {
    }
      
    virtual void HandlePart(const std::map<std::string, std::string>& headers,
                            const void* part,
                            size_t size) ORTHANC_OVERRIDE
    {
      if (dicom_.get())
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NetworkProtocol,
                                        "Multiple DICOM instances were contained in a WADO-RS request");
      }
      else
      {
        dicom_.reset(new Orthanc::ParsedDicomFile(part, size));
        size_ = size;
      }
    }

    Orthanc::ParsedDicomFile* ReleaseDicom()
    {
      if (dicom_.get())
      {
        return dicom_.release();
      }
      else
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NetworkProtocol,
                                        "WADO-RS request didn't contain any DICOM instance");
      }
    }

    size_t GetSize() const
    {
      return size_;
    }
  };

  
  Orthanc::ParsedDicomFile* ParseDicomSuccessMessage::ParseWadoAnswer(
    size_t& fileSize /* OUT */,
    const std::string& answer,
    const std::map<std::string, std::string>& headers)
  {
    std::string contentType, subType, boundary, header;
    if (Orthanc::MultipartStreamReader::GetMainContentType(header, headers) &&
        Orthanc::MultipartStreamReader::ParseMultipartContentType(contentType, subType, boundary, header) &&
        contentType == "multipart/related" &&
        subType == "application/dicom")
    {
      MultipartHandler handler;

      {
        Orthanc::MultipartStreamReader reader(boundary);
        reader.SetHandler(handler);
        reader.AddChunk(answer);
        reader.CloseStream();
      }

      fileSize = handler.GetSize();
      return handler.ReleaseDicom();
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NetworkProtocol,
                                      "Multipart/related answer of application/dicom was expected from DICOMweb server");
    }
  }
}
