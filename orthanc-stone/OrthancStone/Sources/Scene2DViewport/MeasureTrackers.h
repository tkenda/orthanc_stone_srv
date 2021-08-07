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

#include "IFlexiblePointerTracker.h"
#include "../Scene2D/Scene2D.h"
#include "../Scene2D/PointerEvent.h"

#include "MeasureTool.h"
#include "MeasureCommands.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <vector>

namespace OrthancStone
{
  class CreateMeasureTracker : public IFlexiblePointerTracker
  {
  private:
    bool  commitResult_;
    
  protected:
    boost::shared_ptr<CreateMeasureCommand>  command_;
    boost::weak_ptr<IViewport>               viewport_;
    bool                                     alive_;

    /**
    This will return a scoped lock to the viewport.
    If the viewport does not exist anymore, then nullptr is returned.
    */
    IViewport::ILock* GetViewportLock();

    explicit CreateMeasureTracker(boost::weak_ptr<IViewport> viewport);

    virtual ~CreateMeasureTracker();
    
  public:
    virtual void Cancel() ORTHANC_OVERRIDE;
    
    virtual bool IsAlive() const ORTHANC_OVERRIDE;
  };


  class EditMeasureTracker : public IFlexiblePointerTracker
  {
  private:
    ScenePoint2D  originalClickPosition_;
    bool          commitResult_;

  protected:
    boost::shared_ptr<EditMeasureCommand>  command_;
    boost::weak_ptr<IViewport>             viewport_;
    bool                                   alive_;
    
    /**
    This will return a scoped lock to the viewport.
    If the viewport does not exist anymore, then nullptr is returned.
    */
    IViewport::ILock* GetViewportLock();

    EditMeasureTracker(boost::weak_ptr<IViewport> viewport,
                       const PointerEvent& e);

    ~EditMeasureTracker();

    ScenePoint2D GetOriginalClickPosition() const
    {
      return originalClickPosition_;
    }

  public:
    virtual void Cancel() ORTHANC_OVERRIDE;
    
    virtual bool IsAlive() const ORTHANC_OVERRIDE;
  };
}

