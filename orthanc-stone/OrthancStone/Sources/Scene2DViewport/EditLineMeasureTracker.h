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

#include "MeasureTrackers.h"

namespace OrthancStone
{
  class EditLineMeasureCommand;

  class EditLineMeasureTracker : public EditMeasureTracker
  {
  private:
    LineMeasureTool::LineHighlightArea modifiedZone_;

    boost::shared_ptr<EditLineMeasureCommand> GetCommand();

  public:
    /**
    When you create this tracker, you need to supply it with the undo stack
    where it will store the commands that perform the actual measure tool
    creation and modification.
    In turn, a container for these commands to store the actual measuring
    must be supplied, too
    */
    EditLineMeasureTracker(
      boost::shared_ptr<MeasureTool>  measureTool,
      boost::weak_ptr<IViewport>      viewport,
      const PointerEvent&             e);

    virtual void PointerMove(const PointerEvent& e) ORTHANC_OVERRIDE;
    
    virtual void PointerUp(const PointerEvent& e) ORTHANC_OVERRIDE;
    
    virtual void PointerDown(const PointerEvent& e) ORTHANC_OVERRIDE;
  };
}
