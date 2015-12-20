/* regfont.c
 * Temporarily register and unregister fonts under Microsoft(R)
 * Windows(R) 2000 and above.
 * Copyright (c) 2010-2015  David Purton
 */

/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <windows.h>

#include "config.h"

/* Turn on wildcard expansion for the mingw-w64 compiler */
int _dowildcard = -1;

enum REGFONT_TASKS {
  REGFONT_TASK_ADD,
  REGFONT_TASK_REMOVE,
  REGFONT_TASK_HELP,
  REGFONT_TASK_VERSION
} regfont_task;

void addFonts (int n, char **files) {
  int i = 0;
  for ( ; i < n; i++) {
    if (AddFontResource (files[i]) == 0)
      printf ("Error adding %s\n", files[i]);
    else
      printf ("Successfully added font %s\n", files[i]);
  }
  SendMessage (HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
}

void removeFonts (int n, char **files) {
  int i = 0;
  for ( ; i < n; i++) {
    if (RemoveFontResource (files[i]) == 0)
      printf ("Error removing %s\n", files[i]);
    else
      printf ("Successfully removed font %s\n", files[i]);
  }
  SendMessage (HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
}

void printUsage () {
  printf ("Usage: regfont [-a|-r|-h|-v] font1 font2...\n");
  printf ("\t-a, --add\tAdd specified fonts\n");
  printf ("\t-r, --remove\tRemove specified fonts\n");
  printf ("\t-h, --help\tThis help message\n");
  printf ("\t-v, --version\tPrint version information\n");
}

void printVersion () {
  printf ("regfont version %s.\n", VERSION);
  printf ("(c) 2010-2015 David Purton\n");
}

void processOptions (int argc, char **argv) {
  int opt;

  regfont_task = REGFONT_TASK_HELP;

  while (1) {
    int option_index = 0;
    static struct option long_options[] = {
      {"add", 0, 0, 0},
      {"remove", 0, 0, 0},
      {"help", 0, 0, 0},
      {"version", 0, 0, 0},
      {0, 0, 0, 0}
    };

    opt = getopt_long (argc, argv, "arhv", long_options, &option_index);

    if (opt == -1)
      break;

    switch (opt) {
    case 0:
      switch (option_index) {
      case 0: /* add */
        regfont_task = REGFONT_TASK_ADD;
        break;
      case 1: /* remove */
        regfont_task = REGFONT_TASK_REMOVE;
        break;
      case 2: /* help */
        regfont_task = REGFONT_TASK_HELP;
        break;
      case 3: /* version */
        regfont_task = REGFONT_TASK_VERSION;
        break;
      }
      break;
    case 'a':
      regfont_task = REGFONT_TASK_ADD;
      break;
    case 'r':
      regfont_task = REGFONT_TASK_REMOVE;
      break;
    case 'h':
      regfont_task = REGFONT_TASK_HELP;
      break;
    case 'v':
      regfont_task = REGFONT_TASK_VERSION;
      break;
    default:
      break;
    }
  }
}

int main (int argc, char **argv) {
  processOptions (argc, argv);

  switch (regfont_task) {
  case REGFONT_TASK_ADD:
    if (argc - optind > 0) {
      addFonts (argc - optind, &argv[optind]);
    } else {
      printf ("No font files specified!\n");
      printUsage ();
    }
    break;
  case REGFONT_TASK_REMOVE:
    if (argc - optind > 0) {
      removeFonts (argc - optind, &argv[optind]);
    } else {
      printf ("No font files specified!\n");
      printUsage ();
    }
    break;
  case REGFONT_TASK_HELP:
    printUsage ();
    break;
  case REGFONT_TASK_VERSION:
    printVersion ();
    break;
  }

  return 0;
}

