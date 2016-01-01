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

#ifndef TILE_H
#define TILE_H

/* Logging verbosity is between 0 and 5. */

/** Verbosity Level: errors only */
#define GT_ERROR 0 
/** Verbosity Level: notices and warnings */
#define GT_NOTICE 1 
/** Verbosity Level: config */
#define GT_CONFIG 2 
/** Verbosity Level: actions */
#define GT_ACTION 3 
/** Verbosity Level: debug */
#define GT_DEBUG 4 
/** Verbosity Level: trace */
#define GT_TRACE 5 

/**
 * The state structure.
 */
struct tile_state {
        /** Counter for moveResizeWindow. */
        int numwins2;
        /** Counter for recurseWindow. */
        int numwins;
        /** Screen Width. */
        int scrwidth;
        /** Screen Height. */
        int scrheight;
        /** Window frame padding, left. */
        int padleft;
        /** Window frame padding, top. */
        int padtop;
        /** Window frame padding, bottom. */
        int padbottom;
        /** Window frame passing, right. */
        int padright;
        /** Current Desktop/Workspace, for -m and -w. */
        long curdesk;
	/** Iconify all windows (show desktop). */
	char minimize;
};

void dumpConfig(struct tile_config *);
void catch_sig(int sig_num);

#endif
