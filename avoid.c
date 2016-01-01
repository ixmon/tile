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
#include "tilerc.h"
#include "tile.h"
#include "avoid.h"

/**
 * Decides whether or not discovery and placement functions should
 * operate on a window.
 * TODO: automate this with the profile system or similar.
 */
int shouldWeAvoid(Display * dsp, Window win, char *optional_name,
                  struct tile_config *tileconf,
                  struct tile_state *tilestate,
                  char *func)
{
    Atom real_type;
    Atom wm_class;
    Atom work;
    int format;
    unsigned long n, extra;
    int status, i2;
    long *p = 0;
    char atom[AVOIDLINELEN], *value;    //[AVOIDLINELEN];
    unsigned char wmclass[256];

    if (tileconf->verbose >= GT_TRACE)
        printf("shouldWeAvoid: %ld, \"%s\"\n", win,
               optional_name != NULL ? optional_name : "");

    //configurable atom-based avoid
    for (i2 = 0; i2 < tileconf->avoidlistlen; i2++) {
        strcpy(atom, tileconf->avoidlist[i2]);
        value = index(atom, ':');
        value[0] = 0;
        value = &value[2];

        if (tileconf->verbose >= GT_TRACE)
            printf("shouldWeAvoid: %s \"%s\"\n", atom, value);
        if (strcmp(atom, "WM_NAME") == 0) {
            if (optional_name != NULL) {
                //this sub-section is historical; could be deprecated
                if (strcmp(optional_name, value) == 0) {
                    if (tileconf->verbose >= GT_NOTICE)
                        printf("%s->shouldWeAvoid Avoided: %s [WM_NAME]\n", func, optional_name);
                    return TRUE;
                }
            }
        }
        else {
            wm_class = XInternAtom(dsp, atom, False);
            status =
                XGetWindowProperty(dsp, win, wm_class, 0L, 1L, False,
                                   XA_STRING, &real_type, &format, &n,
                                   &extra, (unsigned char **) &wmclass);
            if (!status) {
                if (strcmp((char *) wmclass, value) == 0) {
                    if (tileconf->verbose >= GT_NOTICE)
                        printf("%s->shouldWeAvoid Avoided: %s [%s]\n", func, value, atom);
                    return TRUE;
                }
            }
        }
    }


    //atom-based avoid. avoid windows not on our desktop
    if (tileconf->multi || tileconf->wmulti) {
        char *matom =
            tileconf->multi ? "_WIN_WORKSPACE" : "_NET_WM_DESKTOP";
        work = XInternAtom(dsp, matom, False);
        status =
            XGetWindowProperty(dsp, win, work, 0L, 1L, False, XA_CARDINAL,
                               &real_type, &format, &n, &extra,
                               (unsigned char **) &p);
        if (!status) {
            if (p) {
                if (p[0] != tilestate->curdesk) {
                    if (tileconf->verbose >= GT_NOTICE)
                        printf("%s->shouldWeAvoid Multi-Desktop Avoid: %s [%s != %d]\n",
                               func, optional_name, matom, tilestate->curdesk);
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}
