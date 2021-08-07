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


#include "StringValue.h"

#include <OrthancException.h>
#include <Toolbox.h>

#include <boost/lexical_cast.hpp>


namespace OrthancStone
{
  namespace OsiriX
  {
    void StringValue::ParseVector(Vector& v) const
    {
      size_t a = value_.find('(');
      size_t b = value_.rfind(')');
      if (a == std::string::npos ||
          b == std::string::npos ||
          a >= b)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat,
                                        "Cannot parse vector: " + value_);
      }
      else
      {
        std::vector<std::string> tokens;
        Orthanc::Toolbox::TokenizeString(tokens, value_.substr(a + 1, b - (a + 1)), ',');

        v.resize(tokens.size());
        for (size_t i = 0; i < tokens.size(); i++)
        {
          try
          {
            v[i] = boost::lexical_cast<double>(Orthanc::Toolbox::StripSpaces(tokens[i]));
          }
          catch (boost::bad_lexical_cast&)
          {
            throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat,
                                            "Not a real number: " + tokens[i]);
          }
        }
      }
    }
  }
}
