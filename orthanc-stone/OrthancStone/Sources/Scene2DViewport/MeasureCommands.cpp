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

#include "MeasureCommands.h"

#include <memory>

#include <boost/make_shared.hpp>
#include <boost/ref.hpp>

namespace OrthancStone
{
  IViewport::ILock* MeasureCommand::GetViewportLock()
  {
    boost::shared_ptr<IViewport> viewport = viewport_.lock();
    if (viewport)
      return viewport->Lock();
    else
      return NULL;
  }


  void CreateMeasureCommand::Undo()
  {
    std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
    // simply disable the measure tool upon undo
    GetMeasureTool()->Disable();
    lock->GetController().RemoveMeasureTool(GetMeasureTool());
  }

  void CreateMeasureCommand::Redo()
  {
    std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
    GetMeasureTool()->Enable();
    lock->GetController().AddMeasureTool(GetMeasureTool());
  }

  CreateMeasureCommand::CreateMeasureCommand(boost::weak_ptr<IViewport> viewport)
    : MeasureCommand(viewport)
  {

  }

  CreateMeasureCommand::~CreateMeasureCommand()
  {
    // deleting the command should not change the model state
    // we thus leave it as is
  }

  void DeleteMeasureCommand::Redo()
  {
    std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
    // simply disable the measure tool upon undo
    GetMeasureTool()->Disable();
    lock->GetController().RemoveMeasureTool(GetMeasureTool());
  }

  void DeleteMeasureCommand::Undo()
  {
    std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
    GetMeasureTool()->Enable();
    lock->GetController().AddMeasureTool(GetMeasureTool());
  }

  DeleteMeasureCommand::~DeleteMeasureCommand()
  {
    // deleting the command should not change the model state
    // we thus leave it as is
  }

  DeleteMeasureCommand::DeleteMeasureCommand(boost::shared_ptr<MeasureTool> measureTool,
                                             boost::weak_ptr<IViewport> viewport) :
    MeasureCommand(viewport),
    measureTool_(measureTool),
    mementoModified_(measureTool->CreateMemento()),
    mementoOriginal_(measureTool->CreateMemento())
  {
    std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
    GetMeasureTool()->Disable();
    lock->GetController().RemoveMeasureTool(GetMeasureTool());
  }

  EditMeasureCommand::EditMeasureCommand(boost::shared_ptr<MeasureTool> measureTool,
                                         boost::weak_ptr<IViewport> viewport) :
    MeasureCommand(viewport),
    mementoModified_(measureTool->CreateMemento()),
    mementoOriginal_(measureTool->CreateMemento())
  {
  }

  EditMeasureCommand::~EditMeasureCommand()
  {
  }

  void EditMeasureCommand::Undo()
  {
    // simply disable the measure tool upon undo
    assert(mementoOriginal_.get() != NULL);
    GetMeasureTool()->SetMemento(*mementoOriginal_);
  }

  void EditMeasureCommand::Redo()
  {
    assert(mementoModified_.get() != NULL);
    GetMeasureTool()->SetMemento(*mementoModified_);
  }

  const MeasureToolMemento& EditMeasureCommand::GetMementoOriginal() const
  {
    assert(mementoOriginal_.get() != NULL);
    return *mementoOriginal_;
  }    

  void EditMeasureCommand::SetMementoModified(MeasureToolMemento* memento)
  {
    if (memento == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }
    else
    {
      mementoModified_.reset(memento);
    }
  }

  const MeasureToolMemento& DeleteMeasureCommand::GetMementoOriginal() const
  {
    assert(mementoOriginal_.get() != NULL);
    return *mementoOriginal_;
  }    

  void DeleteMeasureCommand::SetMementoModified(MeasureToolMemento* memento)
  {
    if (memento == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }
    else
    {
      mementoModified_.reset(memento);
    }
  }
}
