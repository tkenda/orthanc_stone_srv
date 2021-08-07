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

#include "DictionaryValue.h"


namespace OrthancStone
{
  namespace OsiriX
  {
    class Annotation : public boost::noncopyable
    {
    private:
      std::string   name_;
      std::string   studyInstanceUid_;
      std::string   seriesInstanceUid_;
      std::string   sopInstanceUid_;

    protected:
      void SetupCommon(const DictionaryValue& dict);
      
    public:
      enum Type
      {
        Type_Angle,
        Type_Line,
        Type_Text
      };

      virtual ~Annotation()
      {
      }

      virtual Type GetType() const = 0;

      const std::string& GetName() const
      {
        return name_;
      }

      const std::string& GetStudyInstanceUid() const
      {
        return studyInstanceUid_;
      }

      const std::string& GetSeriesInstanceUid() const
      {
        return seriesInstanceUid_;
      }

      const std::string& GetSopInstanceUid() const
      {
        return sopInstanceUid_;
      }

      static Annotation* Create(const DictionaryValue& dict);
    };
  }
}
