# Stone of Orthanc
# Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
# Department, University Hospital of Liege, Belgium
# Copyright (C) 2017-2021 Osimis S.A., Belgium
#
# This program is free software: you can redistribute it and/or
# modify it under the terms of the GNU Affero General Public License
# as published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.



#####################################################################
## Load generic parameters
#####################################################################

include(${CMAKE_CURRENT_LIST_DIR}/../../../OrthancStone/Resources/CMake/OrthancStoneParameters.cmake)


#####################################################################
## CMake parameters tunable by the user
#####################################################################

# None for now, but might contain stuff like memory settings


#####################################################################
## Internal CMake parameters to enable the optional subcomponents of
## the Stone of Orthanc
#####################################################################

set(ENABLE_THREADS OFF CACHE INTERNAL "")
