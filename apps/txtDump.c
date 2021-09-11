/*
 Crave SEQ text dumper.
 Copyright (C) 2021  Claudiu Zissulescu

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sequence.h"

static const char *Note(unsigned x)
{
  x = x % 12;
  const char *notes[] =
    { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
  return notes[x];
}

int main (int argc, char *argv[])
{
  sequence_t seq;
  int whichone = 1;
  int convert = 0;
  char *name;

  /* Parse options.  */
  while ((argc > 1) && (argv[1][0] == '-'))
    {
      switch (argv[1][1])
	{
	case 'c':
	  printf ("Parse Crave seq file ...\n");
	  whichone = 1;
	  break;

	case 't':
	  printf ("Parse TD3 seq file ...\n");
	  whichone = 2;
	  break;

	case 'd':
	  printf ("Parse TD3 seq file and convert...\n");
	  whichone = 2;
	  convert = 1;
	  break;

	default:
	  printf("Wrong Argument: %s\n", argv[1]);
	  //usage();
	}

      ++argv;
      --argc;
    }


  if (argc == 1)
    {
      printf ("Expecting a filename.\n");
      return 1;
    }

  name = argv[1];
  if ((whichone == 1) && craveSequence (name, &seq))
    return 1;
  else if ((whichone == 2) && td3Sequence (name, &seq))
    return 1;

  printf ("Swing: %d%%\t Length %d\n", seq.swing, seq.length);
  note_t *notes = seq.notes;
  for (int i = 0; i < seq.length; i++)
    {
      printf ("[%s%d\t%c %c %c\t", Note (notes->note),
	      notes->octave,
	      notes->glide ? 'G' : notes->slide ? 'S' : ' ',
	      notes->accent ? 'A' : ' ',
	      notes->rest ? 'R' : ' ');
      if (whichone == 1)
	printf ("%3d x%d ", notes->velocity, notes->ratchet);

      for (int j = 0; (j < 8) && (whichone == 1); j++)
	if (j <= notes->gate)
	  printf ("#");
	else
	  printf (" ");
      printf (" ]\n");
      notes++;
    }
  printf ("\n");

  if (convert == 1)
    {
      int lenName = 0;
      for (int i = 0; i < strlen (name); i++)
	if (name[i] != '.')
	  lenName ++;
	else
	  break;
      char *dumpName = alloca (lenName + strlen (".crave.seq"));
      memset (dumpName, 0, lenName + 1);
      strncpy (dumpName, name, lenName);
      strcat (dumpName, ".crave.seq");
      dumpCraveSeq (dumpName, &seq);
    }

  return 0;
}
