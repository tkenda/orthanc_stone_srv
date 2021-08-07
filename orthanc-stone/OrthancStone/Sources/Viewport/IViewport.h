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

#include "../Scene2D/ICompositor.h"

namespace OrthancStone
{
  class ViewportController;
  
  /**
   * Class that combines a Scene2D with a canvas where to draw the
   * scene. A call to "Refresh()" will update the content of the
   * canvas. A "IViewport" can possibly be accessed from several
   * threads depending on the rendering back-end (e.g. in SDL or Qt):
   * The "ILock" subclass implements the locking mechanism to modify
   * the content of the scene. 
   *
   * NB: The lock must be a "recursive_mutex", as the viewport
   * controller can lock it a second time (TODO - Why so?).
   **/  
  class IViewport : public boost::noncopyable
  {
  public:
    class ILock : public boost::noncopyable
    {
    public:
      virtual ~ILock()
      {
      }

      virtual bool HasCompositor() const = 0;

      /**
      Do not store the result! Only access the compositor interface through
      the lock.
      */
      virtual ICompositor& GetCompositor() = 0;

      /**
      Do not store the result! Only access the compositor interface through
      the lock.
      */
      virtual ViewportController& GetController() = 0;

      virtual void Invalidate() = 0;


      /**
       * This function must be called when the layout has changed, and
       * thus the size of the canvas must be re-computed. After a call
       * to this method, the canvas size can be accessed through
       * "GetCompositor().GetCanvasWidth()" and
       * "GetCompositor().GetCanvasHeight()". Avoid calling this
       * method too often for performance.
       **/
      virtual void RefreshCanvasSize() = 0;
    };   
    
    virtual ~IViewport()
    {
    }

    virtual ILock* Lock() = 0;
  };
}
