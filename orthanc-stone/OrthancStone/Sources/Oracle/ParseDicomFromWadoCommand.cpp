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


#include "ParseDicomFromWadoCommand.h"

#include <OrthancException.h>

#include <cassert>

namespace OrthancStone
{
  ParseDicomFromWadoCommand::ParseDicomFromWadoCommand(const DicomSource& source,
                                                       const std::string& sopInstanceUid,
                                                       bool transcode,
                                                       Orthanc::DicomTransferSyntax transferSyntax,
                                                       IOracleCommand* restCommand) :
    source_(source),
    sopInstanceUid_(sopInstanceUid),
    transcode_(transcode),
    transferSyntax_(transferSyntax),
    restCommand_(restCommand)
  {
    if (restCommand == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }
    
    if (restCommand_->GetType() != Type_Http &&
        restCommand_->GetType() != Type_OrthancRestApi)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadParameterType);
    }        
  }

  
  IOracleCommand* ParseDicomFromWadoCommand::Clone() const
  {
    assert(restCommand_.get() != NULL);
    return new ParseDicomFromWadoCommand(source_, sopInstanceUid_, transcode_, transferSyntax_, restCommand_->Clone());
  }


  Orthanc::DicomTransferSyntax ParseDicomFromWadoCommand::GetTranscodeTransferSyntax() const
  {
    if (transcode_)
    {
      return transferSyntax_;
    }
    else
    {
      // "IsTranscode()" should have been called
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }
  

  bool ParseDicomFromWadoCommand::IsSameCommand(const ParseDicomFromWadoCommand& other) const
  {
    if (source_.IsSameSource(other.source_) &&
        sopInstanceUid_ == other.sopInstanceUid_ &&
        transcode_ == other.transcode_)
    {
      if (transcode_)
      {
        return transferSyntax_ == other.transferSyntax_;
      }
      else
      {
        return true;
      }
    }
    else
    {
      return false;
    }      
  }    

  
  const IOracleCommand& ParseDicomFromWadoCommand::GetRestCommand() const
  {
    assert(restCommand_.get() != NULL);
    return *restCommand_;
  }


  ParseDicomFromWadoCommand* ParseDicomFromWadoCommand::Create(
    const DicomSource& source,
    const std::string& studyInstanceUid,
    const std::string& seriesInstanceUid,
    const std::string& sopInstanceUid,
    bool transcode,
    Orthanc::DicomTransferSyntax transferSyntax,
    Orthanc::IDynamicObject* payload)
  {
    std::unique_ptr<Orthanc::IDynamicObject> protection(payload);
    
    const std::string uri = ("/studies/" + studyInstanceUid +
                             "/series/" + seriesInstanceUid +
                             "/instances/" + sopInstanceUid);

    std::string s;
    if (transcode)
    {
      s = Orthanc::GetTransferSyntaxUid(transferSyntax);      
    }
    else
    {
      s = "*";  // No transcoding, keep source transfer syntax
    }
    
    std::map<std::string, std::string> arguments, headers;
    headers["Accept"] = ("multipart/related; type=\"application/dicom\"; transfer-syntax=" + s);
                         
    std::unique_ptr<IOracleCommand> rest(
      source.CreateDicomWebCommand(uri, arguments, headers, NULL));

    std::unique_ptr<ParseDicomFromWadoCommand> command(
      new ParseDicomFromWadoCommand(source, sopInstanceUid, transcode, transferSyntax, rest.release()));

    if (protection.get() != NULL)
    {
      command->AcquirePayload(protection.release());
    }
                            
    return command.release();
  }
}
