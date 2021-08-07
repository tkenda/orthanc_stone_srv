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

#include <Enumerations.h>

#include <string>

namespace OrthancStone
{
  class ParseDicomFromWadoCommand : public OracleCommandBase
  {
  private:
    DicomSource                      source_;
    std::string                      sopInstanceUid_;
    bool                             transcode_;
    Orthanc::DicomTransferSyntax     transferSyntax_;
    std::unique_ptr<IOracleCommand>  restCommand_;

  public:
    ParseDicomFromWadoCommand(const DicomSource& source,
                              const std::string& sopInstanceUid,
                              bool transcode,
                              Orthanc::DicomTransferSyntax transferSyntax,
                              IOracleCommand* restCommand);

    virtual Type GetType() const ORTHANC_OVERRIDE
    {
      return Type_ParseDicomFromWado;
    }

    virtual IOracleCommand* Clone() const ORTHANC_OVERRIDE;

    const DicomSource& GetSource() const
    {
      return source_;
    }
    
    const std::string& GetSopInstanceUid() const
    {
      return sopInstanceUid_;
    }

    bool IsTranscode() const
    {
      return transcode_;
    }

    Orthanc::DicomTransferSyntax GetTranscodeTransferSyntax() const;

    bool IsSameCommand(const ParseDicomFromWadoCommand& other) const;
    
    const IOracleCommand& GetRestCommand() const;

    static ParseDicomFromWadoCommand* Create(const DicomSource& source,
                                             const std::string& studyInstanceUid,
                                             const std::string& seriesInstanceUid,
                                             const std::string& sopInstanceUid,
                                             bool transcode,
                                             Orthanc::DicomTransferSyntax transferSyntax,
                                             Orthanc::IDynamicObject* payload);
  };
}
