#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <ctype.h>
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

  if (title)
    printf ("<head><title>/%s</title>", title);
  else
    printf ("<head><title>Brainfuck interpreter</title>");

  printf ("<meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\" />");

  if (title)
    printf ("</head><body><p>Interpreting file: %s</p><pre>\n\n", title);
  else
    printf ("</head><body><p>No file to interpret</p>");
}


void
footer (int exit_status)
{
  printf ("<hr /><p>Brainfuck interpreter completely written in C</p>");
  printf ("<p>Copyright (C) 2010 Jon Anders Skorpen</p><hr />");

  printf ("<p><a href=\"http://validator.w3.org/check?uri=referer\">");
  printf ("<img src=\"http://www.w3.org/Icons/valid-xhtml10\"");
  printf ("alt=\"Valid XHTML 1.0 Strict\" height=\"31\" width=\"88\" /></a></p>");


  printf ("</body></html>");
  exit (exit_status);
}


char *
extract_get_var (char *get, char *name)
{
  char *value = NULL;
  char *tmp;

  int i;

  if (!(tmp = strstr (get, name)))
    return NULL;

  tmp += strlen (name);

  if (*tmp != '=')
    return NULL;

  for (i = 0; *tmp != '&' && *tmp != '\0'; i++)
    tmp++;

  value = malloc (i);

  memcpy (value, tmp-(i-1), i);
  value[i-1] = '\0';

  if (!*value)
    {
      free (value);
      value = NULL;
    }

  return value;
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
  sprintf (valid, "../brainfuck/%s.b", name);

  return valid;
}


int
main (void)
{
  char *filename;
  char *get = getenv ("QUERY_STRING");

  cell = 0;

  filename = extract_get_var (get, "file");
  filename = (valid_filename (filename));

  if (filename)
    header (filename+3);
  else
    header (NULL);

  if (!filename)
    {
      printf ("No such file\n");
      footer (EXIT_FAILURE);
    }

  if ((file = open (filename, O_RDONLY)) == -1)
    {
      printf ("No such file: /%s\n", filename+3);
      footer (EXIT_FAILURE);
    }


  if (!(cells = malloc (30000)))
    {
      printf ("NOT ENOUGH MEMORIES!");
      footer (EXIT_FAILURE);
    }

  memset (cells, 0, 30000);

  i = 0;
  interpret (0);

  printf ("\n\n</pre><hr /><p><a href=\"/%s\">Source code</a></p>", filename+3);

  footer (EXIT_SUCCESS);

  return 0;
}
