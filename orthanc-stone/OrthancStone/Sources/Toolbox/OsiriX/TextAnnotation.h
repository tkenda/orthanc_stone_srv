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

#include "Annotation.h"

#include "../LinearAlgebra.h"  // For "Vector"


namespace OrthancStone
{
  namespace OsiriX
  {
    class TextAnnotation : public Annotation
    {
    private:
      Vector  center_;

    public:
      explicit TextAnnotation(const DictionaryValue& dict);
      
      virtual Type GetType() const ORTHANC_OVERRIDE
      {
        return Type_Text;
      }

      const Vector& GetCenter() const
      {
        return center_;
      }

      const std::string& GetText() const
      {
        return GetName();   // This is just an alias
      }
    };
  }
}