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
#include <shlwapi.h>

#include "config.h"

/* Turn on wildcard expansion for the mingw-w64 compiler */
int _dowildcard = -1;

enum REGFONT_TASKS {
  REGFONT_TASK_ADD,
  REGFONT_TASK_REMOVE,
  REGFONT_TASK_HELP,
  REGFONT_TASK_VERSION
} regfont_task;

typedef enum REGFONT_FONT_TYPES {
  REGFONT_ANY,
  REGFONT_PFB,
  REGFONT_PFM
} regfont_font_type;

enum REGFONT_ERRORS {
  REGFONT_OK,
  REGFONT_INVALID_FONT_PATH,
  REGFONT_FONT_NOT_FOUND,
  REGFONT_FULL_FONT_PATH_TOO_LONG,
  REGFONT_FONT_IS_DIRECTORY,
  REGFONT_NOT_FONT_FILE,
  REGFONT_NOT_POSTSCRIPT
};

int checkFile (char *filename, regfont_font_type type) {
  char fullfilename[MAX_PATH] = "";
  char *fileextension;
  int retval = 0;

  retval = GetFullPathName (filename, MAX_PATH, fullfilename, NULL);

  if (retval > MAX_PATH) {
    fprintf (stderr, "ERROR: Full path for font too long: %s\n", filename);
    return REGFONT_FULL_FONT_PATH_TOO_LONG;
  } else if (retval == 0) {
    fprintf (stderr, "ERROR: Could not get full path for font: %s\n", filename);
    return REGFONT_INVALID_FONT_PATH;
  }

  if (!PathFileExists (fullfilename)) {
    fprintf (stderr, "ERROR: Font not found: %s\n", filename);
    return REGFONT_FONT_NOT_FOUND;
  }

  if (PathIsDirectory (fullfilename)) {
    fprintf (stderr, "ERROR: Font is directory: %s\n", filename);
    return REGFONT_FONT_IS_DIRECTORY;
  }

  fileextension = PathFindExtension (fullfilename);

  if (strlen (fileextension) > 0)
    fileextension++;

  switch (type) {
    case REGFONT_PFM:
      if (CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE,
            fileextension, -1, "pfm", -1) != CSTR_EQUAL) {
        if (CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE,
            fileextension, -1, "pfb", -1) == CSTR_EQUAL) {
          fprintf (stderr, "ERROR: PostScript font specified incorrectly\n");
          fprintf (stderr, "ERROR:     Use \"font.pfm|font.pfb\".\n");
        } else {
          fprintf (stderr, "ERROR: Not a PostScript font file: %s\n", filename);
          fprintf (stderr, "ERROR:     Extension of first file must be pfm\n");
        }
        return REGFONT_NOT_FONT_FILE;
      }
      break;
    case REGFONT_PFB:
      if (CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE,
            fileextension, -1, "pfb", -1) != CSTR_EQUAL) {
        if (CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE,
            fileextension, -1, "pfm", -1) == CSTR_EQUAL) {
          fprintf (stderr, "ERROR: PostScript font specified incorrectly\n");
          fprintf (stderr, "ERROR:     Use \"font.pfm|font.pfb\".\n");
        } else {
          fprintf (stderr, "ERROR: Not a PostScript font file: %s\n", filename);
          fprintf (stderr, "ERROR:     Extension of second file must be pfb\n");
        }
        return REGFONT_NOT_FONT_FILE;
      }
      break;
    case REGFONT_ANY:
    default:
      if (CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE,
            fileextension, -1, "fon", -1) != CSTR_EQUAL &&
          CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE,
            fileextension, -1, "fnt", -1) != CSTR_EQUAL &&
          CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE,
            fileextension, -1, "ttf", -1) != CSTR_EQUAL &&
          CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE,
            fileextension, -1, "ttc", -1) != CSTR_EQUAL &&
          CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE,
            fileextension, -1, "fot", -1) != CSTR_EQUAL &&
          CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE,
            fileextension, -1, "otf", -1) != CSTR_EQUAL &&
          CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE,
            fileextension, -1, "mmm", -1) != CSTR_EQUAL) {
        fprintf (stderr, "ERROR: Not a font file: %s\n", filename);
        fprintf (stderr, "ERROR:     Extension of file must be one of:\n");
        fprintf (stderr, "ERROR:     fon, fnt, ttf, ttc, fot, otf, mmm\n");
        return REGFONT_NOT_FONT_FILE;
      }
      break;
  }

  return REGFONT_OK;
}

int checkPostScriptFile (char *filename) {
  char *pfb_filename;
  char pfm_filename[MAX_PATH];
  int retval;

  pfb_filename = strchr (filename, '|');

  if (!pfb_filename) {
    return REGFONT_NOT_POSTSCRIPT;
  }
  pfb_filename++;

  memset (pfm_filename, 0, sizeof (pfm_filename));
  strncpy (pfm_filename, filename, pfb_filename - filename - 1);

  retval = checkFile (pfm_filename, REGFONT_PFM);
  if (retval != REGFONT_OK)
    return retval;

  return checkFile (pfb_filename, REGFONT_PFB);
}

int checkFontFile (char *filename) {
  int retval;

  retval = checkPostScriptFile (filename);

  if (retval == REGFONT_NOT_POSTSCRIPT)
    return checkFile (filename, REGFONT_ANY);
  else
    return retval;
}

void addFonts (int n, char **files) {
  int i = 0;

  for ( ; i < n; i++) {
    if (checkFontFile (files[i]) == REGFONT_OK) {
      if (AddFontResource (files[i]) == 0)
        fprintf (stderr, "ERROR: Could not add font: %s\n", files[i]);
      else
        printf ("Successfully added font: %s\n", files[i]);
    }
  }

  SendMessage (HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
}

void removeFonts (int n, char **files) {
  int i = 0;

  for ( ; i < n; i++) {
    if (checkFontFile (files[i]) == REGFONT_OK) {
      if (RemoveFontResource (files[i]) == 0)
        fprintf (stderr, "ERROR: Could not remove font: %s\n", files[i]);
      else
        printf ("Successfully removed font: %s\n", files[i]);
    }
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
      fprintf (stderr, "ERROR: No font files specified to add!\n");
      printUsage ();
    }
    break;
  case REGFONT_TASK_REMOVE:
    if (argc - optind > 0) {
      removeFonts (argc - optind, &argv[optind]);
    } else {
      fprintf (stderr, "ERROR: No font files specified to remove!\n");
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

