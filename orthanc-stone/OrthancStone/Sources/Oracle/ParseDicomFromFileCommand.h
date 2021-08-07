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

#include "OracleCommandBase.h"
#include "../Loaders/DicomSource.h"

#include <string>

namespace OrthancStone
{
  class ParseDicomFromFileCommand : public OracleCommandBase
  {
  private:
    DicomSource  source_;
    std::string  path_;
    bool         pixelDataIncluded_;

    ParseDicomFromFileCommand(const ParseDicomFromFileCommand& other) :
      source_(other.source_),
      path_(other.path_),
      pixelDataIncluded_(other.pixelDataIncluded_)
    {
    }

  public:
    ParseDicomFromFileCommand(const DicomSource& source,
                              const std::string& path) :
      source_(source),
      path_(path),
      pixelDataIncluded_(true)
    {
    }

    ParseDicomFromFileCommand(const DicomSource& source,
                              const std::string& dicomDirPath,
                              const std::string& file) :
      source_(source),
      path_(GetDicomDirPath(dicomDirPath, file)),
      pixelDataIncluded_(true)
    {
    }
    
    static std::string GetDicomDirPath(const std::string& dicomDirPath,
                                       const std::string& file);

    virtual Type GetType() const ORTHANC_OVERRIDE
    {
      return Type_ParseDicomFromFile;
    }

    virtual IOracleCommand* Clone() const ORTHANC_OVERRIDE
    {
      return new ParseDicomFromFileCommand(*this);
    }

    const DicomSource& GetSource() const
    {
      return source_;
    }

    const std::string& GetPath() const
    {
      return path_;
    }

    bool IsPixelDataIncluded() const
    {
      return pixelDataIncluded_;
    }

    void SetPixelDataIncluded(bool included)
    {
      pixelDataIncluded_ = included;
    }
  };
}
