/****************************************************************************
 * libs/libnx/nxtk/nxtk_opentoolbar.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdlib.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/nx/nx.h>
#include <nuttx/nx/nxtk.h>

#include "nxtk.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: nxtk_opentoolbar
 *
 * Description:
 *   Create a tool bar at the top of the specified framed window
 *
 * Input Parameters:
 *   hfwnd  - The handle returned by nxtk_openwindow
 *   height - The request height of the toolbar in pixels
 *   cb     - Callbacks used to process toolbar events
 *   arg    - User provided value that will be returned with toolbar
 *            callbacks.
 *
 * Returned Value:
 *   OK on success; ERROR on failure with errno set appropriately
 *
 ****************************************************************************/

int nxtk_opentoolbar(NXTKWINDOW hfwnd, nxgl_coord_t height,
                     FAR const struct nx_callback_s *cb,
                     FAR void *arg)
{
  FAR struct nxtk_framedwindow_s *fwnd =
    (FAR struct nxtk_framedwindow_s *)hfwnd;

#ifdef CONFIG_DEBUG_FEATURES
  if (hfwnd == NULL || cb == NULL || height < 1)
    {
      set_errno(EINVAL);
      return ERROR;
    }
#endif

  /* Initialize the toolbar info */

  fwnd->tbheight = height;
  fwnd->tbcb     = cb;
  fwnd->tbarg    = arg;

  /* Calculate the new dimensions of the toolbar and client windows */

  nxtk_setsubwindows(fwnd);

#ifdef CONFIG_NX_RAMBACKED
  /* The redraw request has no effect if a framebuffer is used with the
   * window.  For that type of window, the application must perform the
   * toolbar update itself and not rely on a redraw notification.
   */

  if (NXBE_ISRAMBACKED(&fwnd->wnd))
    {
      struct nxgl_rect_s relbounds;

      /* Convert to a window-relative bounding box */

      nxgl_rectoffset(&relbounds, &fwnd->wnd.bounds,
                      -fwnd->wnd.bounds.pt1.x, -fwnd->wnd.bounds.pt1.y);

      /* Then re-draw the frame */

      nxtk_drawframe(fwnd, &relbounds); /* Does not fail */
    }
  else
#endif
    {
      /* Redraw the entire window, even the client window must be redrawn
       * because it has changed its vertical position and size.
       */

      nx_redrawreq(&fwnd->wnd, &fwnd->wnd.bounds);
    }

  return OK;
}
