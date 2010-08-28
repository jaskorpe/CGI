#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <ctype.h>
#include <fcntl.h>

#include <unistd.h>

#include <cgi.h>

#include <errno.h>


void interpret (int ignore);
void header (char *title);
void footer (int exit_status);
char *valid_filename (char *name);
void main_site (void);


char *input;
int input_len;

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
          if (input_len--)
            cells[cell] = *(input++);
          else
            {
              printf ("\n\n</pre><hr /><p>Trouble reading from input</p>");
              footer (EXIT_FAILURE);
            }
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
  printf ("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"");
  printf ("\t\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">");

  printf ("<html xmlns=\"http://www.w3.org/1999/xhtml\">");

  if (title)
    printf ("<head><title>/%s</title>", title);
  else
    printf ("<head><title>Brainfuck interpreter</title>");

  printf ("<meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\" />");

  if (title)
    printf ("</head><body><p>Interpreting file: /%s</p>", title);
}


void
footer (int exit_status)
{
  printf ("<hr /><p>Brainfuck interpreter completely written in C</p>");
  printf ("<p>Copyright (C) 2010 Jon Anders Skorpen</p><hr />");

  printf ("<p><a href=\"http://validator.w3.org/check?uri=referer\">");
  printf ("<img src=\"http://www.w3.org/Icons/valid-xhtml10\"");
  printf (" alt=\"Valid XHTML 1.0 Strict\" height=\"31\" width=\"88\" /></a></p>");


  printf ("</body></html>");
  exit (exit_status);
}

char *
valid_filename (char *name)
{
  char *tmp = name;
  char *valid;

  if (!tmp)
    return NULL;

  while (1)
    {
      if (*tmp == '\0')
        break;;

      if (!isupper (*tmp) && !islower (*tmp))
        return NULL;

      tmp++;
    }

  valid = malloc (strlen (name) + 16);

  if (!valid)
    return NULL;

  sprintf (valid, "../brainfuck/%s.b", name);

  return valid;
}


void
main_site (void)
{
  DIR *dp;
  struct dirent *de;
  char *dup_name, *tmp;

  printf ("<p>Welcome to my Brainfuck online interpreter.</p>");

  if (!(dp = opendir ("../brainfuck")))
    {
      printf ("<p>No available files</p>");
      footer (EXIT_FAILURE);
    }

  printf ("<p>Available files in /brainfuck:</p>");

  printf ("<ul>");
  while ((de = readdir (dp)))
    {
      if (de->d_type == DT_UNKNOWN)
        {
          dup_name = tmp = strdup (de->d_name);
          for (; *tmp != '.'; tmp++);
          *tmp = '\0';
          printf ("<li><a href=\"/cgi-bin/brainfuck.cgi?file=%s\">",
                  dup_name);
          printf ("%s</a></li>", de->d_name);
        }
    }

  printf ("</ul>");
}


int
main (void)
{
  char *filename;

  char byte;

  s_cgi *cgi;


  cgi = cgiInit ();

  cgiHeader ();

  if (!(filename = cgiGetValue (cgi, "file")))
    {
      header (NULL);
      main_site ();
      footer (EXIT_SUCCESS);
    }

  cell = 0;

  filename = (valid_filename (filename));

  if (filename)
    header (filename+3);
  else
    header (NULL);

  if (!filename)
    {
      printf ("<p>No such file\n</p>");
      footer (EXIT_FAILURE);
    }

  if ((file = open (filename, O_RDONLY)) == -1)
    {
      printf ("<p>No such file: /%s</p>\n", filename+3);
      footer (EXIT_FAILURE);
    }


  if (!(cells = malloc (30000)))
    {
      printf ("<p>NOT ENOUGH MEMORIES!</p>");
      footer (EXIT_FAILURE);
    }

  memset (cells, 0, 30000);

  if (input = cgiGetValue (cgi, "input"))
    input_len = strlen (input);
  else
    input_len = 0;

  i = 0;

  printf ("<p>Output:</p><pre>\n\n");
  interpret (0);

  printf ("\n\n</pre><hr /><p><a href=\"/%s\">Source code</a>:</p>", filename+3);


  lseek (file, 0, SEEK_SET);

  printf ("<pre>");
  while (read (file, &byte, 1))
    fwrite (&byte, 1, 1, stdout);

  printf ("</pre>");

  footer (EXIT_SUCCESS);

  return 0;
}
