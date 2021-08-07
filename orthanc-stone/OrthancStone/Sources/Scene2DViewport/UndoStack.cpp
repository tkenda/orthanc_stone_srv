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

#include "UndoStack.h"

#include "MeasureCommands.h"

#include "../StoneException.h"

namespace OrthancStone
{
  UndoStack::UndoStack() : numAppliedCommands_(0)
  {}

  void UndoStack::PushCommand(boost::shared_ptr<MeasureCommand> command)
  {
    commandStack_.erase(
      commandStack_.begin() + numAppliedCommands_,
      commandStack_.end());

    ORTHANC_ASSERT(std::find(commandStack_.begin(), commandStack_.end(), command)
      == commandStack_.end(), "Duplicate command");
    commandStack_.push_back(command);
    numAppliedCommands_++;
  }

  void UndoStack::Undo()
  {
    ORTHANC_ASSERT(CanUndo(), "");
    commandStack_[numAppliedCommands_ - 1]->Undo();
    numAppliedCommands_--;
  }

  void UndoStack::Redo()
  {
    ORTHANC_ASSERT(CanRedo(), "");
    commandStack_[numAppliedCommands_]->Redo();
    numAppliedCommands_++;
  }

  bool UndoStack::CanUndo() const
  {
    return numAppliedCommands_ > 0;
  }

  bool UndoStack::CanRedo() const
  {
    return numAppliedCommands_ < commandStack_.size();
  }

}
