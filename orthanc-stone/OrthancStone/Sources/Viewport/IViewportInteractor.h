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

#include "IViewport.h"
#include "../Scene2DViewport/IFlexiblePointerTracker.h"

namespace OrthancStone
{
  class IViewportInteractor : public boost::noncopyable
  {
  public:
    virtual ~IViewportInteractor()
    {
    }

    virtual IFlexiblePointerTracker* CreateTracker(boost::weak_ptr<IViewport> viewport,
                                                   const PointerEvent& event,
                                                   unsigned int viewportWidth,
                                                   unsigned int viewportHeight) = 0;

    virtual bool HasMouseHover() const = 0;

    virtual void HandleMouseHover(IViewport& viewport,
                                  const PointerEvent& event) = 0;
  };
}
