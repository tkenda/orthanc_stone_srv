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
#  error The macro ORTHANC_ENABLE_DCMTK must be defined
#endif

#if ORTHANC_ENABLE_DCMTK != 1
#  error Support for DCMTK must be enabled to use ParseDicomFromFileCommand
#endif

#include "OracleCommandBase.h"
#include "../Messages/IMessageEmitter.h"
#include "../Messages/IObserver.h"

#include <map>

namespace Orthanc
{
  class ParsedDicomFile;
}

namespace OrthancStone
{
  class DicomSource;
  
  class ParseDicomSuccessMessage : public OriginMessage<OracleCommandBase>
  {
    ORTHANC_STONE_MESSAGE(__FILE__, __LINE__);
    
  private:
    const DicomSource&         source_;
    Orthanc::ParsedDicomFile&  dicom_;
    size_t                     fileSize_;
    bool                       hasPixelData_;
    
  public:
    ParseDicomSuccessMessage(const OracleCommandBase& command,
                             const DicomSource& source,
                             Orthanc::ParsedDicomFile& dicom,
                             size_t fileSize,
                             bool hasPixelData) :
      OriginMessage(command),
      source_(source),
      dicom_(dicom),
      fileSize_(fileSize),
      hasPixelData_(hasPixelData)
    {
    }

    const DicomSource& GetSource() const
    {
      return source_;
    }

    Orthanc::ParsedDicomFile& GetDicom() const
    {
      return dicom_;
    }

    size_t GetFileSize() const
    {
      return fileSize_;
    }

    bool HasPixelData() const
    {
      return hasPixelData_;
    }
    
    static Orthanc::ParsedDicomFile* ParseWadoAnswer(size_t& fileSize /* OUT */,
                                                     const std::string& answer,
                                                     const std::map<std::string, std::string>& headers);
  };
}
