#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>

#include <unistd.h>


unsigned char *cells;
unsigned int cell;
int i;
int file;


void
interpret (int ignore)
{
  int file_pos = i;
  char instruction;


  for (; read (file, &instruction, 1); i++)
    {
      if (ignore)
        {
          if (instruction == '[')
            {
              i++;
              interpret (ignore);
              continue;
            }
          else if (instruction == ']')
            {
              return;
            }
          else
            {
              continue;
            }
        }

      switch (instruction)
        {
        case '>':
          if (cell++ == 29999)
            cell = 0;
          break;
        case '<':
          if (cell-- == 0)
            cell = 29999;
          break;
        case '+':
          cells[cell]++;
          break;
        case '-':
          cells[cell]--;
          break;
        case '.':
          printf ("%c", cells[cell]);
          break;
        case ',':
          break;
        case '[':
          if (cells[cell])
            {
              i++;
              interpret (0);
            }
          else
            {
              i++;
              interpret (1);
            }
          break;
        case ']':
          if (cells[cell])
            {
              lseek (file, file_pos, SEEK_SET);
              i = file_pos-1;
            }
          else
            {
              return;
            }
          break;
        }
    }
}


void
header (char *title)
{
  printf ("Content-type: text/html\n\n");


  printf ("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"");
  printf ("\t\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">");

  printf ("<html xmlns=\"http://www.w3.org/1999/xhtml\">");
  printf ("<head><title>%s</title>", title);

  printf ("<meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\" />");
  printf ("</head><body><pre>");
}


void
footer (int exit_status)
{
  printf ("</pre><hr />");
  printf ("</body></html>");
  exit (exit_status);
}


int
main (void)
{
  char *filename = getenv ("QUERY_STRING");

  cell = 0;

  header (filename);

  if ((file = open (filename, O_RDONLY)) == -1)
    {
      printf ("No such file: %s\n", filename);
      footer (-1);
    }


  if (!(cells = malloc (30000)))
    {
      printf ("NOT ENOUGH MEMORIES!");
      footer (-1);
    }

  memset (cells, 0, 30000);

  i = 0;
  interpret (0);

  footer (0);

  return 0;
}
