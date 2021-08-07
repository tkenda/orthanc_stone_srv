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

#include "MeasureTool.h"

#include <Logging.h>
#include <Enumerations.h>
#include <OrthancException.h>

#include <boost/shared_ptr.hpp>
#include <boost/math/constants/constants.hpp>

#include "../Viewport/IViewport.h"

namespace OrthancStone
{
  void MeasureTool::Enable()
  {
    enabled_ = true;
    RefreshScene();
  }

  void MeasureTool::Disable()
  {
    enabled_ = false;
    RefreshScene();
  }

  bool MeasureTool::IsEnabled() const
  {
    return enabled_;
  }

  IViewport::ILock* MeasureTool::GetViewportLock()
  {
    boost::shared_ptr<IViewport> viewport = viewport_.lock();
    if (viewport)
      return viewport->Lock();
    else
      return NULL;
  }

  IViewport::ILock* MeasureTool::GetViewportLock() const
  {
    boost::shared_ptr<IViewport> viewport = viewport_.lock();
    if (viewport)
      return viewport->Lock();
    else
      return NULL;
  }

  MeasureTool::MeasureTool(boost::weak_ptr<IViewport> viewport) :
    enabled_(true),
    viewport_(viewport)
  {
  }

  void MeasureTool::PostConstructor()
  {
    std::unique_ptr<IViewport::ILock> lock(GetViewportLock());

    if (lock.get() != NULL)
    {
      Register<ViewportController::SceneTransformChanged>(
        lock->GetController(), 
        &MeasureTool::OnSceneTransformChanged);
    }
  }

  bool MeasureTool::IsSceneAlive() const
  {
    // since the lifetimes of the viewport, viewportcontroller (and the
    // measuring tools inside it) are linked, the scene is alive as 
    // long as the viewport is alive (which is technically not the case
    // during its dtor)
    std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
    return (lock != NULL);
  }

  void MeasureTool::OnSceneTransformChanged(
    const ViewportController::SceneTransformChanged& message)
  {
    RefreshScene();
  }
}
