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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
CONFIG VALUES:

orientation vertical|horizontal
multi-desktop netwm|workspace|off
wmprofile <profile-name>
verbosity 0-5
avoid Atom(STRING) value
*/

#include "tilerc.h"
#include "tile.h"

/**
 * Parse the named configuration file.
 */
void parseRC(char *file, struct tile_config *tileconf)
{
    char buf[128], buf2[128], avoid_val[128];
    char *tokpt;
    int slen;
    FILE *fp;

    fp = fopen(file, "r");
    if (fp != NULL) {
        if (tileconf->verbose >= GT_CONFIG)
            printf("parseConfig config: %s [rc]\n", file);
        //lie for consistency. this func called only by parseConfig
    }
    else {
//              printf("ERROR: Could not open file %s\n", file);
        return;
    }

    while (!feof(fp)) {
        int param = CFG_NULL, avc = 0;
        fgets(buf, 128, fp);

        if (strlen(buf) < 2)
            continue;
        else if (buf[0] == '#')
            continue;

        tokpt = strtok(buf, " ");
        if (strcmp(tokpt, "orientation") == 0) {
            param = CFG_ORIEN;
        }
        else if (strcmp(tokpt, "multi-desktop") == 0) {
            param = CFG_MDESK;
        }
        else if (strcmp(tokpt, "wmprofile") == 0) {
            param = CFG_WPROF;
        }
        else if (strcmp(tokpt, "verbosity") == 0) {
            param = CFG_VERBO;
        }
        else if (strcmp(tokpt, "avoid") == 0) {
            param = CFG_AVOID;
        }

        sprintf(buf2, "");
        sprintf(avoid_val, "");
        while ((tokpt = strtok(NULL, " ")) != NULL) {
            if (param == CFG_AVOID) {
                avc++;
                switch (avc) {
                case 1:
                    sprintf(buf2, "%s", strtrim(tokpt));
                    break;
                default:
                    if (avc != 2)
                        sprintf(avoid_val, "%s %s", avoid_val,
                                strtrim(tokpt));
                    else
                        sprintf(avoid_val, "%s", strtrim(tokpt));
                }
            }
            else {
                sprintf(buf2, "%s", strtrim(tokpt));
                break;
            }
        }

        switch (param) {
        case CFG_NULL:
            puts("ERROR: Invalid configuration parameter");
            //TODO: save param name, then printf here
            break;
        case CFG_ORIEN:
            if (tileconf->verbose >= GT_CONFIG)
                printf("parseRC set orientation: %s\n", buf2);
            if (strcmp(buf2, "vertical") == 0)
                tileconf->orientation = VERTICAL;
            else if (strcmp(buf2, "horizontal") == 0)
                tileconf->orientation = HORIZONTAL;
            break;
        case CFG_MDESK:
            if (tileconf->verbose >= GT_CONFIG)
                printf("parseRC set multi-desktop style: %s\n", buf2);
            if (strcmp(buf2, "netwm") == 0)
                tileconf->wmulti = TRUE;
            else if (strcmp(buf2, "workspace") == 0)
                tileconf->multi = TRUE;
            else if (strcmp(buf2, "off") == 0) {
                tileconf->multi = FALSE;
                tileconf->wmulti = FALSE;
            }
            break;
        case CFG_WPROF:
            if (tileconf->verbose >= GT_CONFIG)
                printf("parseRC set window manager profile: %s\n", buf2);
            tileconf->profile_name = malloc(strlen(buf2));
            strcpy(tileconf->profile_name, buf2);
            break;
        case CFG_VERBO:
            tileconf->verbose = atoi(buf2);
            if (tileconf->verbose >= GT_CONFIG)
                printf("parseRC set verbosity level: %s\n", buf2);

            break;
        case CFG_AVOID:
            slen = strlen(buf2) + strlen(avoid_val) + 2;
            if (tileconf->avoidlistlen >= 128)
                continue;       //array size limited
            tileconf->avoidlist[tileconf->avoidlistlen] =
                (char *) malloc(sizeof(char) * slen);
            sprintf(tileconf->avoidlist[tileconf->avoidlistlen], "%s: %s",
                    buf2, avoid_val);
//                  tileconf->avoidlist[tileconf->avoidlistlen][slen] = 0;

            if (tileconf->verbose >= GT_CONFIG)
                printf("parseRC added avoid criteria: \"%s\"\n",
                       tileconf->avoidlist[tileconf->avoidlistlen]);
            tileconf->avoidlistlen++;
            break;
        }
    }
    fclose(fp);

    return;
}

/**
 * Find and parse configuration (rc) files.
 */
void parseConfig(struct tile_config *tileconf)
{
    char conf[256], i = 0;
    char localconf[128];

    sprintf(conf, "%s/share/tile/rc", INSTPFX);
    sprintf(localconf, "%s/.tile", getenv("HOME"));

    //set some defaults
    tileconf->avoidlist = (char **) malloc(sizeof(char) * 128);
    tileconf->scpadt = 0;
    tileconf->scpadr = 0;
    tileconf->scpadb = 0;
    tileconf->scpadl = 0;
    tileconf->avoidlistlen = 0;

  readavoid:
    i++;
    parseRC(conf, tileconf);
    if (i == 1) {
        sprintf(conf, "%s/rc", localconf);
        goto readavoid;
    }
}

/**
 * Find and parse wmprofiles files.
 */
void parseProfiles(struct tile_config *tileconf)
{
    char conf[256], set = FALSE, i = 0;
    char localconf[128];
    FILE *prof;

    sprintf(conf, "%s/share/tile/wmprofiles", INSTPFX);
    sprintf(localconf, "%s/.tile", getenv("HOME"));

  readconf:
    i++;
    prof = fopen(conf, "r");
    if (prof == NULL)
        goto endoffunc;
    else if (tileconf->verbose >= GT_CONFIG)
        printf("parseProfiles config: %s [wmprofiles]\n", conf);
    while (!feof(prof)) {
        char pn[25];
        int t, r, b, l;
        fscanf(prof, "%s %d %d %d %d\n", pn, &t, &r, &b, &l);
        if (pn[0] == '#') {
            continue;
        }
        else if (strcmp(pn, tileconf->profile_name) == 0) {
            tileconf->scpadt = t;
            tileconf->scpadr = r;
            tileconf->scpadb = b;
            tileconf->scpadl = l;
            set = TRUE;
            break;
        }
    }
    fclose(prof);

  endoffunc:
    if (!set && i == 1) {
        sprintf(conf, "%s/wmprofiles", localconf);
        goto readconf;
    }
}

/**
 * Trim left and right whitespace from string.
 */
char *strtrim(char *string)
{
    return ltrim(rtrim(string));
}

/**
 * Trim left whitespace from string.
 */
char *ltrim(char *string)
{
    int len, i, found = FALSE;
    char *ret = (char *) malloc(strlen(string));
    if ((len = strlen(string)) == 0)
        return string;
    else {
        for (i = 0; i < len; i++) {
            if (string[i] != '\n' && string[i] != ' ' && string[i] != '\t') {
                found = TRUE;
                break;
            }
        }
        if (found)
            sprintf(ret, "%s", &string[i]);
        else
            sprintf(ret, "");   //whole string is whitespace
    }
    return ret;
}

/**
 * Trim right whitespace from string.
 */
char *rtrim(char *string)
{
    int len, i, found = FALSE;
    char *ret = (char *) malloc(strlen(string));
    if ((len = strlen(string)) == 0)
        return string;
    else {
        for (i = len - 1; i >= 0; i--) {
            if (string[i] != '\n' && string[i] != ' ' && string[i] != '\t') {
                found = TRUE;   //found non-whitespace
                break;
            }
        }
        sprintf(ret, "%s", string);
        if (found)
            i++;
        ret[i] = 0;
    }
    return ret;
}
