/**
 * Stone of Orthanc
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017-2021 Osimis S.A., Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/


#pragma once

/**
 * This file serves as an indirection to avoid large "#if
 * ORTHANC_BUILDING_STONE_LIBRARY == 1" in "WebAssemblyOracle.cpp"
 **/

#include "WebAssemblyOracle.h"

#include "../../../OrthancStone/Sources/Oracle/OracleCommandExceptionMessage.h"

#if ORTHANC_ENABLE_DCMTK == 1
#  include "../../../OrthancStone/Sources/Oracle/ParseDicomSuccessMessage.h"
#endif

#include "../../../OrthancStone/Sources/Oracle/GetOrthancImageCommand.h"
#include "../../../OrthancStone/Sources/Oracle/GetOrthancWebViewerJpegCommand.h"
#include "../../../OrthancStone/Sources/Oracle/HttpCommand.h"
#include "../../../OrthancStone/Sources/Oracle/OrthancRestApiCommand.h"
#include "../../../OrthancStone/Sources/Oracle/ParseDicomFromWadoCommand.h"
#include "../../../OrthancStone/Sources/Oracle/SleepOracleCommand.h"
