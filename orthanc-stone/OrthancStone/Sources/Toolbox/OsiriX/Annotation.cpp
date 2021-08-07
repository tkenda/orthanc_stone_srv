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


#include "Annotation.h"

#include "AngleAnnotation.h"
#include "IntegerValue.h"
#include "LineAnnotation.h"
#include "StringValue.h"
#include "TextAnnotation.h"

#include <Logging.h>

namespace OrthancStone
{
  namespace OsiriX
  {
    void Annotation::SetupCommon(const DictionaryValue& dict)
    {
      const IValue* value = dict.LookupValue("Name");
      if (value == NULL)
      {
        name_.clear();
      }
      else
      {
        name_ = dynamic_cast<const StringValue&>(*value).GetValue();
      }
    
      value = dict.LookupValue("StudyInstanceUID");
      if (value == NULL)
      {
        studyInstanceUid_.clear();
      }
      else
      {
        studyInstanceUid_ = dynamic_cast<const StringValue&>(*value).GetValue();
      }
    
      value = dict.LookupValue("SeriesInstanceUID");
      if (value == NULL)
      {
        seriesInstanceUid_.clear();
      }
      else
      {
        seriesInstanceUid_ = dynamic_cast<const StringValue&>(*value).GetValue();
      }

      value = dict.LookupValue("SOPInstanceUID");
      if (value == NULL)
      {
        sopInstanceUid_.clear();
      }
      else
      {
        sopInstanceUid_ = dynamic_cast<const StringValue&>(*value).GetValue();
      }
    }
    

    Annotation* Annotation::Create(const DictionaryValue& dict)
    {
      const IntegerValue& type = dynamic_cast<const IntegerValue&>(dict.GetValue("Type"));

      switch (type.GetValue())
      {
        case 5:
          return new LineAnnotation(dict, false);

        case 12:
          return new AngleAnnotation(dict);

        case 13:
          return new TextAnnotation(dict);

        case 14:
          return new LineAnnotation(dict, true);

        default:
          LOG(WARNING) << "Unsupported OsiriX annotation type: " << type.GetValue();
          return NULL;
      }
    }
  }
}
