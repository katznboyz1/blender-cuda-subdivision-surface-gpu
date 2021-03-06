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
 */

/** \file
 * \ingroup pygen
 *
 * This file contains wrapper functions related to global interpreter lock.
 * these functions are slightly different from the original Python API,
 * don't throw SIGABRT even if the thread state is NULL. */

#include <Python.h>

#include "../BPY_extern.h"
#include "BLI_utildefines.h"

/* analogue of PyEval_SaveThread() */
BPy_ThreadStatePtr BPY_thread_save(void)
{
  /* Use `_PyThreadState_UncheckedGet()` instead of `PyThreadState_Get()`, to avoid a fatal error
   * issued when a thread state is NULL (the thread state can be NULL when quitting Blender).
   *
   * `PyEval_SaveThread()` will release the GIL, so this thread has to have the GIL to begin with
   * or badness will ensue. */
  if (_PyThreadState_UncheckedGet() && PyGILState_Check()) {
    return (BPy_ThreadStatePtr)PyEval_SaveThread();
  }
  return NULL;
}

/* analogue of PyEval_RestoreThread() */
void BPY_thread_restore(BPy_ThreadStatePtr tstate)
{
  if (tstate) {
    PyEval_RestoreThread((PyThreadState *)tstate);
  }
}
