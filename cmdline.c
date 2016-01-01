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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getopt.h"
#include "tilerc.h"
#include "cmdline.h"
#include "tile.h"

/** Print software package name and version. */
void cmdline_parser_print_version(void)
{
    printf("%s %s\n", CMDLINE_PARSER_PACKAGE, CMDLINE_PARSER_VERSION);
}

/** Print program usage. */
void cmdline_parser_print_help(void)
{
    cmdline_parser_print_version();
    printf("\n" "Usage: %s [OPTIONS]...\n", CMDLINE_PARSER_PACKAGE);
    printf("              --help            Print help and exit\n");
    printf("   -V         --version         Print version and exit\n");
    printf("   -d         --dump            Dump run-time config and exit\n");
    printf
        ("              --verbose         Increase Verbosity (up to 5 times)\n\n");
    printf("   -h         --horizontal      Horizontal Placement\n");
    printf
        ("   -v         --vertical        Vertical Placement (default)\n\n");
    printf
        ("   -pSTRING   --profile=STRING  Window Manager Profile Name (default='Default')\n\n");
    printf
        ("   -m         --win-workspace   Enable multi-desktop _WIN_WORKSPACE support (oroborus, fluxbox, etc) (default=off)\n");
    printf
        ("   -w         --net-wm          Enable multi-desktop _NET_*_DESKTOP support (GNOME, NET WM, etc) (default=off)\n");
}


static char *gengetopt_strdup(const char *s);

/**
 * strdup.c replacement of strdup, which is not standard.
 */
char *gengetopt_strdup(const char *s)
{
    char *result = (char *) malloc(strlen(s) + 1);
    if (result == (char *) 0)
        return (char *) 0;
    strcpy(result, s);
    return result;
}

/** Parse the command-line options. */
int cmdline_parser(int argc, char *const *argv, struct tile_config *tileconf)   //struct gengetopt_args_info *args_info)
{
    int c;                      /* Character of the parsed option.  */

    tileconf->orientation = VERTICAL;
    tileconf->verbose = FALSE;
    tileconf->multi = FALSE;
    tileconf->wmulti = FALSE;
    tileconf->profile_name = "Default";

    parseConfig(tileconf);

    optarg = 0;
    optind = 1;
    opterr = 1;
    optopt = '?';

    while (1) {
        int option_index = 0;
        char *stop_char;

        static struct option long_options[] = {
            {"help", 0, NULL, 0},
            {"horizontal", 0, NULL, 'h'},
            {"version", 0, NULL, 'V'},
            {"vertical", 0, NULL, 'v'},
            {"profile", 1, NULL, 'p'},
            {"dump", 0, NULL, 'd'},
            {"win-workspace", 0, NULL, 'm'},
            {"net-wm", 0, NULL, 'w'},
            {"verbose", 0, NULL, 0},
            {NULL, 0, NULL, 0}
        };

        stop_char = 0;
        c = getopt_long(argc, argv, "hVvp:mw", long_options,
                        &option_index);

        if (c == -1)
            break;              /* Exit from `while (1)' loop.  */

        switch (c) {
        case 'h':              /* Horizontal Placement.  */
            tileconf->orientation = HORIZONTAL;
            break;
        
        case 'd':
            dumpConfig(tileconf);
            break;

        case 'V':              /* Print version and exit.  */
            cmdline_parser_print_version();
            exit(EXIT_SUCCESS);

        case 'v':              /* Vertical Placement.  */
            tileconf->orientation = VERTICAL;
            break;

        case 'p':              /* Window Manager Profile Name.  */
/*            if (strcmp(tileconf->profile_name, "Default") != 0) {
                fprintf(stderr,
                        "%s: `--profile' (`-p') option given more than once\n",
                        CMDLINE_PARSER_PACKAGE);
                exit(EXIT_FAILURE);
            } */
            tileconf->profile_name = gengetopt_strdup(optarg);
            break;

        case 'm':              /* Enable multi-desktop _WIN_WORKSPACE support (oroborus, fluxbox, etc).  */
            if (tileconf->wmulti) {
                fprintf(stderr,
                        "%s: (Workspace) `-m' and (Net-WM) `-w' conflict\n",
                        CMDLINE_PARSER_PACKAGE);
                exit(EXIT_FAILURE);
            }
            tileconf->multi = TRUE;
            break;

        case 'w':              /* Enable multi-desktop _NET_*_DESKTOP support (GNOME, NET WM, etc).  */
            if (tileconf->multi) {
                fprintf(stderr,
                        "%s: (Net-WM) `-w' and (Workspace) `-m' conflict\n",
                        CMDLINE_PARSER_PACKAGE);
                exit(EXIT_FAILURE);
            }
            tileconf->wmulti = TRUE;
            break;

        case 0:                /* Long option with no short option */
            /* Increase Verbosity.  */
            if (strcmp(long_options[option_index].name, "verbose") == 0) {
                if (tileconf->verbose == 5) {
                    fprintf(stderr,
                            "%s: Too much verbosity! The limit is five, which will\n",
                            CMDLINE_PARSER_PACKAGE);
                    fprintf(stderr,
                            "       output thousands of messages.\n");
                    exit(EXIT_FAILURE);

                }
                tileconf->verbose++;
                break;
            }
            else if (strcmp(long_options[option_index].name, "help") == 0) {
                cmdline_parser_print_help();
                exit(EXIT_SUCCESS);
            }

        case '?':              /* Invalid option.  */
            /* `getopt_long' already printed an error message.  */
            exit(EXIT_FAILURE);

        default:               /* bug: option not considered.  */
            fprintf(stderr, "%s: option unknown: %c\n",
                    CMDLINE_PARSER_PACKAGE, c);
            abort();
        }                       /* switch */
    }                           /* while */

    return 0;
}
