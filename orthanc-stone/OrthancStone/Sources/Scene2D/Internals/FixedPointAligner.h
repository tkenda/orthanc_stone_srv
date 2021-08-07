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

#include "../../Scene2DViewport/PredeclaredTypes.h"
#include "../../Scene2D/ScenePoint2D.h"
#include "../../Viewport/IViewport.h"

#include <boost/weak_ptr.hpp>

namespace OrthancStone
{
  namespace Internals
  {
    // During a mouse event that modifies the view of a scene, keeps
    // one point (the pivot) at a fixed position on the canvas
    class FixedPointAligner : public boost::noncopyable
    {
    private:
      boost::weak_ptr<IViewport> viewport_;
      ScenePoint2D               pivot_;
      ScenePoint2D               canvas_;

      /**
      This will return a scoped lock to the viewport.
      If the viewport does not exist anymore, then nullptr is returned.
      */
      IViewport::ILock* GetViewportLock();

    public:
      FixedPointAligner(boost::weak_ptr<IViewport> viewport,
                        const ScenePoint2D& p);

      void Apply();
    };
  }
}
