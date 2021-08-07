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

/**
 * Besides the "pragma once" above that only protects this file,
 * define a macro to prevent including different versions of
 * "OrthancStone.h"
 **/
#ifndef __ORTHANC_STONE_H
#define __ORTHANC_STONE_H

#include <OrthancFramework.h>

#if ORTHANC_ENABLE_OPENGL == 1
#  define GL_GLEXT_PROTOTYPES 1
#endif


#endif /* __ORTHANC_STONE_H */
