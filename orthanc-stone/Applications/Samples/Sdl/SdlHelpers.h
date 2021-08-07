/**
 * Stone of Orthanc
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017-2021 Osimis S.A., Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/


#pragma once

#if ORTHANC_ENABLE_SDL != 1
# error This file cannot be used if ORTHANC_ENABLE_SDL != 1
#endif

#include "../../Platforms/Sdl/SdlViewport.h"

#include <boost/shared_ptr.hpp>

#include <SDL.h>

#include <map>
#include <string>

namespace OrthancStoneHelpers
{

  inline OrthancStone::KeyboardModifiers GetKeyboardModifiers(const uint8_t* keyboardState,
                                                              const int scancodeCount)
  {
    using namespace OrthancStone;
    int result = KeyboardModifiers_None;

    if (keyboardState != NULL)
    {
      if (SDL_SCANCODE_LSHIFT < scancodeCount &&
          keyboardState[SDL_SCANCODE_LSHIFT])
      {
        result |= KeyboardModifiers_Shift;
      }

      if (SDL_SCANCODE_RSHIFT < scancodeCount &&
          keyboardState[SDL_SCANCODE_RSHIFT])
      {
        result |= KeyboardModifiers_Shift;
      }

      if (SDL_SCANCODE_LCTRL < scancodeCount &&
          keyboardState[SDL_SCANCODE_LCTRL])
      {
        result |= KeyboardModifiers_Control;
      }

      if (SDL_SCANCODE_RCTRL < scancodeCount &&
          keyboardState[SDL_SCANCODE_RCTRL])
      {
        result |= KeyboardModifiers_Control;
      }

      if (SDL_SCANCODE_LALT < scancodeCount &&
          keyboardState[SDL_SCANCODE_LALT])
      {
        result |= KeyboardModifiers_Alt;
      }

      if (SDL_SCANCODE_RALT < scancodeCount &&
          keyboardState[SDL_SCANCODE_RALT])
      {
        result |= KeyboardModifiers_Alt;
      }
    }

    return static_cast<KeyboardModifiers>(result);
  }


  inline void GetPointerEvent(OrthancStone::PointerEvent& p,
                              const OrthancStone::ICompositor& compositor,
                              SDL_Event event,
                              const uint8_t* keyboardState,
                              const int scancodeCount)
  {
    using namespace OrthancStone;
    KeyboardModifiers modifiers = GetKeyboardModifiers(keyboardState, scancodeCount);

    switch (event.button.button)
    {
    case SDL_BUTTON_LEFT:
      p.SetMouseButton(OrthancStone::MouseButton_Left);
      break;

    case SDL_BUTTON_RIGHT:
      p.SetMouseButton(OrthancStone::MouseButton_Right);
      break;

    case SDL_BUTTON_MIDDLE:
      p.SetMouseButton(OrthancStone::MouseButton_Middle);
      break;

    default:
      p.SetMouseButton(OrthancStone::MouseButton_None);
      break;
    }

    p.AddPosition(compositor.GetPixelCenterCoordinates(event.button.x, event.button.y));
    p.SetAltModifier( (modifiers & KeyboardModifiers_Alt) != 0);
    p.SetControlModifier( (modifiers & KeyboardModifiers_Control) != 0);
    p.SetShiftModifier( (modifiers & KeyboardModifiers_Shift) != 0);
  }

  
  inline boost::shared_ptr<OrthancStone::SdlViewport> GetSdlViewportFromWindowId(
    const std::vector<boost::shared_ptr<OrthancStone::SdlViewport> >& viewports,
    Uint32 windowID)
  {
    using namespace OrthancStone;
    for (size_t i = 0; i < viewports.size(); ++i)
    {
      boost::shared_ptr<IViewport> viewport = viewports[i];
      boost::shared_ptr<SdlViewport> sdlViewport = boost::dynamic_pointer_cast<SdlViewport>(viewport);
      Uint32 curWindowID = sdlViewport->GetSdlWindowId();
      if (windowID == curWindowID)
        return sdlViewport;
    }
    
    return boost::shared_ptr<OrthancStone::SdlViewport>();
  }
}


