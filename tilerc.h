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
 
#ifndef TILERC_H
#define TILERC_H

/** Fallback configuration option (non-existent). */
#define CFG_NULL 0
/** Orientation configuration option. */
#define CFG_ORIEN 1
/** Multi-Desk configuration option. */
#define CFG_MDESK 2
/** wmprofile configuration option. */
#define CFG_WPROF 3
/** Verbosity level configuration option. */
#define CFG_VERBO 4
/** Avoid data configuration option. */
#define CFG_AVOID 5
/** True */
#define TRUE 1
/** False */
#define FALSE 0
/** Horizontal Window Placement. */
#define HORIZONTAL 2
/** Vertical Window Placement. */
#define VERTICAL 4

/** The configuration object. */
struct tile_config {
        /** Window Orientation. */
        int orientation;
        
        /** Verbosity Level. */
        char verbose;
        
        /** Workspace Multi-Desk Boolean. */
        char multi;
        /** Net-WM Multi-Desk Boolean. */
        char wmulti;
        
        /** Window Manager Profile Name. */
        char *profile_name;

        /** Screen virtual box top, initialized from wmprofile. */
        int scpadt;
        /** Screen virtual box right, initialized from wmprofile. */
        int scpadr;
        /** Screen virtual box bottom, initialized from wmprofile. */
        int scpadb;
        /** Screen virtual box left, initialized from wmprofile. */
        int scpadl;
        
        /** List of window avoid criteria. */
        char **avoidlist;
        /** Length of avoid criteria array. */
        int avoidlistlen;
};

char *ltrim(char *);
char *rtrim(char *);
char *strtrim(char *);
void parseConfig(struct tile_config*);
void parseProfiles(struct tile_config*);
void parseRC(char *, struct tile_config*);

#endif
