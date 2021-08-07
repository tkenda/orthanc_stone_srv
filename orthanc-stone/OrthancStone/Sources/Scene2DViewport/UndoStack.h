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

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <vector>

namespace OrthancStone
{
  class MeasureCommand;

  class UndoStack : public boost::noncopyable
  {
  public:
    UndoStack();

    /**
    Stores a command :
    - this first trims the undo stack to keep the first numAppliedCommands_
    - then it adds the supplied command at the top of the undo stack

    In other words, when a new command is pushed, all the undone (and not
    redone) commands are removed.
    */
    void PushCommand(boost::shared_ptr<MeasureCommand> command);

    /**
    Undoes the command at the top of the undo stack, or throws if there is no
    command to undo.
    You can check "CanUndo" first to protect against extraneous redo.
    */
    void Undo();

    /**
    Redoes the command that is just above the last applied command in the undo
    stack or throws if there is no command to redo.
    You can check "CanRedo" first to protect against extraneous redo.
    */
    void Redo();

    /** selfexpl */
    bool CanUndo() const;

    /** selfexpl */
    bool CanRedo() const;
  
  private:
    std::vector<boost::shared_ptr<MeasureCommand> > commandStack_;

    /**
    This is always between >= 0 and <= undoStack_.size() and gives the
    position where the controller is in the undo stack.
    - If numAppliedCommands_ > 0, one can undo
    - If numAppliedCommands_ < numAppliedCommands_.size(), one can redo
    */
    size_t                      numAppliedCommands_;
  };
}
