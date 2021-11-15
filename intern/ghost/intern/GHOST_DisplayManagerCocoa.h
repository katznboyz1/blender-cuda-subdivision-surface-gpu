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
 * Declaration of GHOST_DisplayManagerCocoa class.
 */

#pragma once

#ifndef __APPLE__
#  error Apple only!
#endif  // __APPLE__

#include "GHOST_DisplayManager.h"

/**
 * Manages system displays  (Mac OSX/Cocoa implementation).
 * \see GHOST_DisplayManager
 */
class GHOST_DisplayManagerCocoa : public GHOST_DisplayManager {
 public:
  /**
   * Constructor.
   */
  GHOST_DisplayManagerCocoa(void);

  /**
   * Returns the number of display devices on this system.
   * \param numDisplays: The number of displays on this system.
   * \return Indication of success.
   */
  GHOST_TSuccess getNumDisplays(uint8_t &numDisplays) const;

  /**
   * Returns the number of display settings for this display device.
   * \param display: The index of the display to query with 0 <= display < getNumDisplays().
   * \param numSetting: The number of settings of the display device with this index.
   * \return Indication of success.
   */
  GHOST_TSuccess getNumDisplaySettings(uint8_t display, int32_t &numSettings) const;

  /**
   * Returns the current setting for this display device.
   * \param display: The index of the display to query with 0 <= display < getNumDisplays().
   * \param index: The setting index to be returned.
   * \param setting: The setting of the display device with this index.
   * \return Indication of success.
   */
  GHOST_TSuccess getDisplaySetting(uint8_t display,
                                   int32_t index,
                                   GHOST_DisplaySetting &setting) const;

  /**
   * Returns the current setting for this display device.
   * \param display: The index of the display to query with 0 <= display < getNumDisplays().
   * \param setting: The current setting of the display device with this index.
   * \return Indication of success.
   */
  GHOST_TSuccess getCurrentDisplaySetting(uint8_t display, GHOST_DisplaySetting &setting) const;

  /**
   * Changes the current setting for this display device.
   * \param display: The index of the display to query with 0 <= display < getNumDisplays().
   * \param setting: The current setting of the display device with this index.
   * \return Indication of success.
   */
  GHOST_TSuccess setCurrentDisplaySetting(uint8_t display, const GHOST_DisplaySetting &setting);

 protected:
  // Do not cache values as OS X supports screen hot plug
  /** Cached number of displays. */
  // CGDisplayCount m_numDisplays;
  /** Cached display id's for each display. */
  // CGDirectDisplayID* m_displayIDs;
};