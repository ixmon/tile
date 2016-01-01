/*
 * Copyright (c) 2005 Greg Schenzel
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <stdio.h>
#include <stdlib.h>
#include "tilerc.h"
#include "tile.h"
#include "x11.h"
#include "avoid.h"

/**
 * Find window geometry element.
 * @param val One of GWIDTH, GHEIGHT, GX, GY
 * @return Geometry element
 */
int getGeom(Display * dpy, Window win, int val)
{
    XWindowAttributes winatt;
    if (!XGetWindowAttributes(dpy, win, &winatt)) {
        fprintf(stderr, "ERROR: XGetWindowAttributes failed.\n");
        exit(1);
    }

    switch (val) {
    case GWIDTH:
        return winatt.width;
    case GHEIGHT:
        return winatt.height;
    case GX:
        return winatt.x;
    case GY:
        return winatt.y;
    }
    return -1;
}

/**
 * Goes up the window tree for a top-level window (with decorations).
 */
Window onRoot(Display * dsp, Window win)
{
    //int i;
    Window parent_win;
    Window root_win;
    Window twin = win;
    Window *child_windows;
    unsigned int num_child_windows;

    while (1) {
        XQueryTree(dsp, twin, &root_win, &parent_win, &child_windows,
                   &num_child_windows);

        if (parent_win == root_win)
            break;
        else
            twin = parent_win;
    }
    return twin;
}

/**
 * Recurse through windows, doing initial window list calculations.
 */
void recurseWindow(Display * dsp, Window win, struct tile_config *tileconf,
                   struct tile_state *tilestate)
{
    int i;
    Window parent_win;
    Window root_win;
    Window *child_windows;
    unsigned int num_child_windows;

    if (tileconf->verbose >= GT_TRACE)
        printf("recurseWindow: %ld\n", win);

    XQueryTree(dsp, win, &root_win, &parent_win, &child_windows,
               &num_child_windows);
    for (i = 0; i < num_child_windows; i++) {
        recurseWindow(dsp, child_windows[i], tileconf, tilestate);
        if (isNotIconified(dsp, child_windows[i])) {
            if (tileconf->verbose >= GT_DEBUG)
                printf("recurseWindow Visible: %ld\n", child_windows[i]);
            XTextProperty tpr;
            if (0 != XGetWMName(dsp, child_windows[i], &tpr)) {
                if (!shouldWeAvoid
                    (dsp, child_windows[i], (char *) tpr.value, tileconf,
                     tilestate, "recurseWindow")) {
                    tilestate->numwins++;
                    if (tileconf->verbose >= GT_NOTICE)
                        printf("recurseWindow Approved: %s\n", tpr.value);
                    if (tilestate->padleft == -1) {     //wm decoration characteristics uninitialized
                        Window onroot = onRoot(dsp, child_windows[i]);
                        int rx, ry, rw, rh, ax, ay, aw, ah;
                        //window, with decorations values
                        rx = getGeom(dsp, onroot, GX);
                        ry = getGeom(dsp, onroot, GY);
                        rw = getGeom(dsp, onroot, GWIDTH);
                        rh = getGeom(dsp, onroot, GHEIGHT);
                        //window, without decorations values
                        ax = getGeom(dsp, child_windows[i], GX);
                        ay = getGeom(dsp, child_windows[i], GY);
                        aw = getGeom(dsp, child_windows[i], GWIDTH);
                        ah = getGeom(dsp, child_windows[i], GHEIGHT);
                        //logical conclusions
                        tilestate->padleft = ax - rx;
                        tilestate->padright =
                            (rw - tilestate->padleft) - aw;
                        tilestate->padtop = ay - ry;
                        tilestate->padbottom =
                            (rh - tilestate->padtop) - ah;
                        if (tileconf->verbose >= GT_CONFIG)
                            printf
                                ("Window Manager Frame: l%d r%d t%d b%d\n",
                                 tilestate->padleft, tilestate->padright,
                                 tilestate->padtop, tilestate->padbottom);
                    }
                    /*                    realloc(winids, numwins*sizeof(Window));
                       winids[numwins-1] = child_windows[i];
                       fprintf(stdout, "NOT ICON: %s\n", tpr.value); */
                }
            }
        }
    }
    XFree(child_windows);
}

/**
 * Recurse through windows, doing resize and move operations.
 */
void moveResizeWindow(Display * dsp, Window win,
                      struct tile_config *tileconf,
                      struct tile_state *tilestate)
{
    int i;
    Window parent_win;
    Window root_win;
    Window *child_windows;
    unsigned int num_child_windows;

    XQueryTree(dsp, win, &root_win, &parent_win, &child_windows,
               &num_child_windows);
    for (i = 0; i < num_child_windows; i++) {
        moveResizeWindow(dsp, child_windows[i], tileconf, tilestate);
        if (isNotIconified(dsp, child_windows[i])) {
            XTextProperty tpr;
            if (0 != XGetWMName(dsp, child_windows[i], &tpr)) {
                if (!shouldWeAvoid
                    (dsp, child_windows[i], (char *) tpr.value, tileconf,
                     tilestate, "moveResizeWindow")) {
                    if (tileconf->verbose >= GT_ACTION)
                        printf("moveResizeWindow: %s\n", tpr.value);
                    int x, y, width, height;
		  if (!tilestate->minimize) {
                    switch (tileconf->orientation) {
                    case HORIZONTAL:
                        width =
                            (tilestate->scrwidth - tilestate->padleft) -
                            tilestate->padright;
                        height =
                            ((tilestate->scrheight / tilestate->numwins) -
                             tilestate->padtop) - tilestate->padbottom;
                        x = 0 + tileconf->scpadl;
                        y = (tilestate->numwins2 *
                             (height + tilestate->padtop +
                              tilestate->padbottom)) + tileconf->scpadt;
                        break;
                    case VERTICAL:
                        width =
                            ((tilestate->scrwidth / tilestate->numwins) -
                             tilestate->padleft) - tilestate->padright;
                        height =
                            (tilestate->scrheight - tilestate->padtop) -
                            tilestate->padbottom;
                        x = (tilestate->numwins2 *
                             (width + tilestate->padleft +
                              tilestate->padright)) + tileconf->scpadl;
                        y = 0 + tileconf->scpadt;
                        break;
                    }
                    XMoveResizeWindow(dsp, child_windows[i], x, y, width,
                                      height);
		  }
		  else {
		    XIconifyWindow(dsp, child_windows[i], 0); 
		  }
		  tilestate->numwins2++;
                }
            }
        }
    }
    XFree(child_windows);
}

/**
 * Checks whether a window is visible.
 */
int isNotIconified(Display * dsp, Window win)
{
    Atom real_type;
    Atom wm_state;
    int format;
    unsigned long n, extra;
    int status;
    long *p = 0;

    wm_state = XInternAtom(dsp, "WM_STATE", False);
    status =
        XGetWindowProperty(dsp, win, wm_state, 0L, 1L, False, wm_state,
                           &real_type, &format, &n, &extra,
                           (unsigned char **) &p);
    if (!status) {
        if (p)
            return (p[0] == NormalState) ? 1 : 0;
        else
            return 0;
    }
}
