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
#include "cmdline.h"
#include <libgen.h>

/** Entry point. */
int main(int argc, char **argv)
{
    Display *dsp;
    Window win;
    char * invocation = basename(argv[0]);
    struct tile_config *tileconf;
    struct tile_state *tilestate;

    tileconf = (struct tile_config *) malloc(sizeof(struct tile_config));
    tilestate = (struct tile_state *) malloc(sizeof(struct tile_state));

    cmdline_parser(argc, argv, tileconf);

    //signal(11, catch_sig); //catch SIGSEGV

    if ((dsp = XOpenDisplay("")) == NULL) {
        fputs("ERROR: Could not connect to X11 Server.\n", stderr);
        exit(1);
    }

    win = DefaultRootWindow(dsp);

    tilestate->minimize = strcmp(invocation, "iconify") == 0;
    tilestate->numwins2 = 0;
    tilestate->numwins = 0;
    tilestate->padleft = -1;
    tilestate->curdesk = 0;

    if (tileconf->wmulti || tileconf->multi) {
        Atom real_type;
        Atom work;
        int format;
        unsigned long n, extra;
        int status;
        long *p = 0;
        work =
            XInternAtom(dsp,
                        tileconf->
                        multi ? "_WIN_WORKSPACE" : "_NET_CURRENT_DESKTOP",
                        False);
        status =
            XGetWindowProperty(dsp, win, work, 0L, 1L, False, XA_CARDINAL,
                               &real_type, &format, &n, &extra,
                               (unsigned char **) &p);
        if (!status) {
            if (p) {
                tilestate->curdesk = p[0];
            }
        }
    }

    parseProfiles(tileconf);

    tilestate->scrwidth =
        getGeom(dsp, win, GWIDTH) - (tileconf->scpadl + tileconf->scpadr);
    tilestate->scrheight =
        getGeom(dsp, win, GHEIGHT) - (tileconf->scpadt + tileconf->scpadb);

    if (tileconf->verbose >= GT_ACTION)
        puts("recurseWindow (pass 1): find windows for count");
    recurseWindow(dsp, win, tileconf, tilestate);

    if (tileconf->verbose >= GT_ACTION)
        puts("moveResizeWindow (pass 2): move and resize windows");
    moveResizeWindow(dsp, win, tileconf, tilestate);

    return 0;
}

/** Catch a kill signal. */
void catch_sig(int sig_num)
{
    /* re-set the signal handler again to catch_int, for next time */
    //    signal(SIGINT, catch_int);
    puts("ERROR: Exiting, caught signal!");
    exit(1);
}

/** Dump the program configuration and exit. */
void dumpConfig(struct tile_config *tileconf)
{
    printf("Orientation: %s\n",
           tileconf->orientation == VERTICAL ? "vertical" : "horizontal");
    printf("Verbosity Level: %d\n", tileconf->verbose);
    printf("Multi-Desk Workspace: %s\n",
           tileconf->multi ? "True" : "False");
    printf("Multi-Desk Net-WM: %s\n", tileconf->wmulti ? "True" : "False");
    printf("WM Profile: %s\n", tileconf->profile_name);
    exit(0);
}
