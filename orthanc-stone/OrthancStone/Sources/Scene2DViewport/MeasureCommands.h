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

#include "../Viewport/IViewport.h"

// to be moved into Stone
#include "PredeclaredTypes.h"
#include "MeasureTool.h"
#include "LineMeasureTool.h"
#include "AngleMeasureTool.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace OrthancStone
{
  class MeasureCommand : public boost::noncopyable
  {
  protected:
    boost::weak_ptr<IViewport> viewport_;

    /**
    This will return a scoped lock to the viewport.
    If the viewport does not exist anymore, then nullptr is returned.
    */
    IViewport::ILock* GetViewportLock();

  public:
    explicit MeasureCommand(boost::weak_ptr<IViewport> viewport) :
      viewport_(viewport)
    {
    }

    virtual ~MeasureCommand()
    {
    }
    
    virtual void Undo() = 0;

    virtual void Redo() = 0;
  };

  
  class CreateMeasureCommand : public MeasureCommand
  {
  private:
    /** Must be implemented by the subclasses that create the actual tool */
    virtual boost::shared_ptr<MeasureTool> GetMeasureTool() = 0;

  public:
    explicit CreateMeasureCommand(boost::weak_ptr<IViewport> viewport);
    
    virtual ~CreateMeasureCommand();
    
    virtual void Undo() ORTHANC_OVERRIDE;
    
    virtual void Redo() ORTHANC_OVERRIDE;
  };

  
  class EditMeasureCommand : public MeasureCommand
  {
  private:
    /** Must be implemented by the subclasses that edit the actual tool */
    virtual boost::shared_ptr<MeasureTool> GetMeasureTool() = 0;

    /** This memento is updated by the subclasses upon modifications */
    std::unique_ptr<MeasureToolMemento> mementoModified_;
    
    /** This memento is the original object state */
    std::unique_ptr<MeasureToolMemento> mementoOriginal_;

  protected:
    void SetMementoModified(MeasureToolMemento* memento);  // takes ownership

  public:
    EditMeasureCommand(boost::shared_ptr<MeasureTool> measureTool,
                       boost::weak_ptr<IViewport> viewport);

    virtual ~EditMeasureCommand();

    virtual void Undo() ORTHANC_OVERRIDE;

    virtual void Redo() ORTHANC_OVERRIDE;

    const MeasureToolMemento& GetMementoOriginal() const;
  };

  
  class DeleteMeasureCommand : public MeasureCommand
  {
  private:
    /** Must be implemented by the subclasses that edit the actual tool */
    virtual boost::shared_ptr<MeasureTool> GetMeasureTool()
    {
      return measureTool_;
    }

    boost::shared_ptr<MeasureTool> measureTool_;

    /** This memento is updated by the subclasses upon modifications */
    std::unique_ptr<MeasureToolMemento> mementoModified_;

    /** This memento is the original object state */
    std::unique_ptr<MeasureToolMemento> mementoOriginal_;

  protected:
    void SetMementoModified(MeasureToolMemento* memento);  // takes ownership

  public:
    DeleteMeasureCommand(boost::shared_ptr<MeasureTool> measureTool,
                         boost::weak_ptr<IViewport> viewport);

    virtual ~DeleteMeasureCommand();
    
    virtual void Undo() ORTHANC_OVERRIDE;
    
    virtual void Redo() ORTHANC_OVERRIDE;

    const MeasureToolMemento& GetMementoOriginal() const;
  };
}

