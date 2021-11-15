/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 */

/** \file
 * \ingroup GHOST
 * Declaration of GHOST_EventTrackpad class.
 */

#pragma once

#include "GHOST_Event.h"

/**
 * Trackpad (scroll, magnify, rotate, ...) event.
 */
class GHOST_EventTrackpad : public GHOST_Event {
 public:
  /**
   * Constructor.
   * \param msec: The time this event was generated.
   * \param window: The window of this event.
   * \param subtype: The subtype of the event.
   * \param x: The x-delta of the pan event.
   * \param y: The y-delta of the pan event.
   */
  GHOST_EventTrackpad(uint64_t msec,
                      GHOST_IWindow *window,
                      GHOST_TTrackpadEventSubTypes subtype,
                      int32_t x,
                      int32_t y,
                      int32_t deltaX,
                      int32_t deltaY,
                      bool isDirectionInverted)
      : GHOST_Event(msec, GHOST_kEventTrackpad, window)
  {
    m_trackpadEventData.subtype = subtype;
    m_trackpadEventData.x = x;
    m_trackpadEventData.y = y;
    m_trackpadEventData.deltaX = deltaX;
    m_trackpadEventData.deltaY = deltaY;
    m_trackpadEventData.isDirectionInverted = isDirectionInverted;
    m_data = &m_trackpadEventData;
  }

 protected:
  /** The mouse pan data */
  GHOST_TEventTrackpadData m_trackpadEventData;
};