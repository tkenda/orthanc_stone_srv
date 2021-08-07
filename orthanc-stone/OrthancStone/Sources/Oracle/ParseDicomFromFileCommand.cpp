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


#include "ParseDicomFromFileCommand.h"

#include <OrthancException.h>

#include <boost/filesystem/path.hpp>

namespace OrthancStone
{
  std::string ParseDicomFromFileCommand::GetDicomDirPath(const std::string& dicomDirPath,
                                                         const std::string& file)
  {
    std::string tmp = file;

#if !defined(_WIN32)
    std::replace(tmp.begin(), tmp.end(), '\\', '/');
#endif

    boost::filesystem::path base = boost::filesystem::path(dicomDirPath).parent_path();

    return (base / tmp).string();
  }
}
