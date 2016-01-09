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

int regfont_debugging = 0;

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
  REGFONT_NOT_POSTSCRIPT,
  REGFONT_POSTSCRIPT_FONT_SPECIFIED_INCORRECTLY,
  REGFONT_MISMATCHED_POSTSCRIPT_FILES
};

void dbprintf (const char *fmt, ...) {
  if (regfont_debugging) {
    va_list ap;
    va_start (ap, fmt);
    fprintf (stderr, "DEBUG: ");
    vfprintf (stderr, fmt, ap);
    fprintf (stderr, "\n");
    fflush (stderr);
    va_end (ap);
  }
}

int checkFile (char *filename, regfont_font_type type) {
  char fullfilename[MAX_PATH] = "";
  char *fileextension;
  int retval = 0;

  dbprintf ("    Checking file...");

  dbprintf ("    Getting full path...");
  retval = GetFullPathName (filename, MAX_PATH, fullfilename, NULL);
  dbprintf ("    Full path: %s", fullfilename);

  if (retval > MAX_PATH) {
    fprintf (stderr, "ERROR: Full path for font too long: %s\n", filename);
    return REGFONT_FULL_FONT_PATH_TOO_LONG;
  } else if (retval == 0) {
    fprintf (stderr, "ERROR: Could not get full path for font: %s\n", filename);
    return REGFONT_INVALID_FONT_PATH;
  }

  dbprintf ("    Checking if file exists...");
  if (!PathFileExists (fullfilename)) {
    fprintf (stderr, "ERROR: Font not found: %s\n", filename);
    return REGFONT_FONT_NOT_FOUND;
  }
  dbprintf ("    File %s found", filename);

  dbprintf ("    Checking if file is a directory...");
  if (PathIsDirectory (fullfilename)) {
    fprintf (stderr, "ERROR: Font is directory: %s\n", filename);
    return REGFONT_FONT_IS_DIRECTORY;
  }
  dbprintf ("    File is not a directory");

  dbprintf ("    Getting file extension...");
  fileextension = PathFindExtension (fullfilename);
  dbprintf ("    File extension found: %s", fileextension);

  if (strlen (fileextension) > 0)
    fileextension++;

  dbprintf ("    Checking if file is a font...");
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
        return REGFONT_POSTSCRIPT_FONT_SPECIFIED_INCORRECTLY;
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
        return REGFONT_POSTSCRIPT_FONT_SPECIFIED_INCORRECTLY;
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

  dbprintf ("    File is a font");
  dbprintf ("    Completed checking file");

  return REGFONT_OK;
}

int checkPostScriptFile (char *filename) {
  char *pipe_pos;
  char pfb_filename[MAX_PATH];
  char pfm_filename[MAX_PATH];
  int retval;

  dbprintf ("    Checking for PostScript font...");

  pipe_pos = strchr (filename, '|');
  if (!pipe_pos) {
    dbprintf ("    Not a PostScript font (no '|' character found)");
    return REGFONT_NOT_POSTSCRIPT;
  }
  dbprintf ("    PostScript font found ('|' character found)");

  pipe_pos++;

  dbprintf ("    Extracting pfm file...");
  memset (pfm_filename, 0, sizeof (pfm_filename));
  strncpy (pfm_filename, filename, pipe_pos - filename - 1);
  dbprintf ("    pfm file: %s", pfm_filename);

  dbprintf ("    Extracting pfb file...");
  memset (pfb_filename, 0, sizeof (pfb_filename));
  strncpy (pfb_filename, pipe_pos, strlen(pipe_pos));
  dbprintf ("    pfb file: %s", pfb_filename);

  retval = checkFile (pfm_filename, REGFONT_PFM);
  if (retval != REGFONT_OK) {
    dbprintf ("    PostScript font check complete");
    return retval;
  }

  retval = checkFile (pfb_filename, REGFONT_PFB);
  if (retval != REGFONT_OK) {
    dbprintf ("    PostScript font check complete");
    return retval;
  }

  dbprintf ("    Checking if pfm matches pfb...");
  PathStripPath (pfm_filename);
  PathRemoveExtension (pfm_filename);
  PathStripPath (pfb_filename);
  PathRemoveExtension (pfb_filename);
  if (CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE,
            pfm_filename, -1, pfb_filename, -1) != CSTR_EQUAL) {
    fprintf (stderr, "ERROR: PostScript font specified incorrectly\n");
    fprintf (stderr, "ERROR:     pfm and pfb filenames must match (%s != %s)\n",
        pfm_filename, pfb_filename);
    dbprintf ("    PostScript font check complete");
    return REGFONT_MISMATCHED_POSTSCRIPT_FILES;
  }
  dbprintf ("    pfm file matches pfb file");

  dbprintf ("    PostScript font check complete");
  return retval;
}

int checkFontFile (char *filename) {
  int retval;

  dbprintf ("    Checking font...");

  retval = checkPostScriptFile (filename);

  if (retval == REGFONT_NOT_POSTSCRIPT)
    retval = checkFile (filename, REGFONT_ANY);

  dbprintf ("    Font check complete");
  return retval;
}

void addFonts (int n, char **files) {
  int i = 0;

  dbprintf ("Adding fonts: Starting");
  for ( ; i < n; i++) {
    dbprintf ("Trying to add font: %s", files[i]);
    if (checkFontFile (files[i]) == REGFONT_OK) {
      dbprintf ("    Adding font to system font table...");
      if (AddFontResource (files[i]) == 0)
        fprintf (stderr, "ERROR: Adding %s to system font table failed\n",
            files[i]);
      else
        printf ("Successfully added font: %s\n", files[i]);
    }
  }
  dbprintf ("Adding fonts: Finished");

  dbprintf ("Sending font broadcast change message");
  SendMessage (HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
  dbprintf ("Font change broadcast message sent");
}

void removeFonts (int n, char **files) {
  int i = 0;

  dbprintf ("Removing fonts: Starting");
  for ( ; i < n; i++) {
    dbprintf ("Trying to remove font: %s", files[i]);
    if (checkFontFile (files[i]) == REGFONT_OK) {
      dbprintf ("    Removing font from system font table...");
      if (RemoveFontResource (files[i]) == 0)
        fprintf (stderr, "ERROR: Removing %s from system font table failed\n",
            files[i]);
      else
        printf ("Successfully removed font: %s\n", files[i]);
    }
  }
  dbprintf ("Removing fonts: Finished");

  dbprintf ("Sending font change broadcast message");
  SendMessage (HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
  dbprintf ("Font change broadcast message sent");
}

void printUsage () {
  dbprintf ("Printing usage");
  printf ("Usage: regfont [-a|-r|-h|-v|-d] font1 font2...\n");
  printf ("\t-a, --add\tAdd specified fonts\n");
  printf ("\t-r, --remove\tRemove specified fonts\n");
  printf ("\t-h, --help\tThis help message\n");
  printf ("\t-v, --version\tPrint version information\n");
  printf ("\t-d, --debug\tTurn on debugging information\n");
  dbprintf ("Printing usage: Finished");
}

void printVersion () {
  dbprintf ("Printing version");
  printf ("regfont version %s.\n", VERSION);
  printf ("(c) 2010-2015 David Purton\n");
  dbprintf ("Printing version: Finished");
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
      {"debug", 0, 0, 0},
      {0, 0, 0, 0}
    };

    opt = getopt_long (argc, argv, "arhvd", long_options, &option_index);

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
      case 4: /* debug */
        regfont_debugging = -1;
        dbprintf ("Processing options: Turning on debugging");
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
    case 'd':
      regfont_debugging = -1;
      dbprintf ("Processing options: Turning on debugging");
      break;
    default:
      break;
    }
  }

  if (regfont_debugging) {
    fprintf (stderr, "DEBUG: Processing options: Commandline arguments found:\n");
    for (int i = 0; i < argc; i++) {
      fprintf (stderr, "DEBUG:     %s\n", argv[i]);
    }
    fflush (stderr);
    switch (regfont_task) {
      case REGFONT_TASK_ADD:
        dbprintf ("Processing options: Task selected: Add fonts");
        break;
      case REGFONT_TASK_REMOVE:
        dbprintf ("Processing options: Task selected: Remove fonts");
        break;
      case REGFONT_TASK_HELP:
        dbprintf ("Processing options: Task selected: Print usage");
        break;
      case REGFONT_TASK_VERSION:
        dbprintf ("Processing options: Task selected: Print version");
        break;
      default:
        dbprintf ("Processing options: No task selected");
        break;
    }
    if (optind < argc) {
      fprintf (stderr, "DEBUG: Processing options: Font files to process:\n");
      int i = optind;
      while (i < argc) {
        fprintf (stderr, "DEBUG:     %s\n", argv[i++]);
      }
      fflush (stderr);
    }
  }

  dbprintf("Processing options: Finished");
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

