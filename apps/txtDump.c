/*
 Crave SEQ text dumper.
 Copyright (C) 2020  Claudiu Zissulescu

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
#include "CraveFile.h"

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

  if (argc == 1)
    {
      printf ("Expecting a filename.\n");
      return 1;
    }

  if (readSequence (argv[1], &seq))
    return 1;

  printf ("Swing: %d%%\t Length %d\n", seq.swing, seq.length);
  note_t *notes = seq.notes;
  for (int i = 0; i < seq.length; i++)
    {
      printf ("[%s%d\t%c %c %c\t", Note (notes->note),
	      notes->octave,
	      notes->glide ? 'G' : ' ',
	      notes->accent ? 'A' : ' ',
	      notes->rest ? 'R' : ' ');
      printf ("%3d x%d ", notes->velocity, notes->ratchet);

      for (int j = 0; j < 8; j++)
	if (j <= notes->gate)
	  printf ("#");
	else
	  printf (" ");
      printf (" ]\n");
      notes++;
    }
  printf ("\n");

  return 0;
}
