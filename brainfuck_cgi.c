/* Copyright (C) 2010 Jon Anders Skorpen
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <sys/time.h>

#include <ctype.h>
#include <fcntl.h>

#include <unistd.h>

#include <cgi.h>

#include <errno.h>

#include <sys/mman.h>


void interpret (int ignore);
void header (char *title);
void footer (int exit_status);
char *valid_filename (char *name);
void main_site (void);


struct timeval stop_time;
struct timeval start_time;
struct timeval offset = {5, 0};

char *input;
int input_len;

unsigned char *cells;
unsigned int cell;

char *code = 0;
char *code_start;
char *code_end;
int code_len;

int i;

int file;


void
interpret (int ignore)
{
  int code_pos = i;
  char instruction;


  for (; code < code_end; i++)
    {

      gettimeofday (&start_time, NULL);

      if (timercmp (&start_time, &stop_time, >))
        {
          printf ("</pre><p>Timeout!</p>");
          footer (EXIT_FAILURE);
        }

      instruction = *(code++);

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
              code = code_start+code_pos;
              i = code_pos-1;
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
    printf ("<head><title>%s</title>", title);
  else
    printf ("<head><title>Brainfuck interpreter</title>");

  printf ("<meta http-equiv=\"content-type\" ");
  printf ("content=\"text/html;charset=utf-8\" />");
  printf ("</head><body>");
}


void
footer (int exit_status)
{
  printf ("<hr /><p>Brainfuck interpreter completely written in C</p>");
  printf ("<p>Copyright (C) 2010 Jon Anders Skorpen</p><hr />");

  printf ("<p>Get source code <a href=\"http://github.com/jaskorpe/CGI\">");
  printf ("here</a>.</p><hr /><p>Version: 1.0</p><hr />\n");

  printf ("<p><a href=\"http://validator.w3.org/check?uri=referer\">");
  printf ("<img src=\"http://www.w3.org/Icons/valid-xhtml10\"");
  printf (" alt=\"Valid XHTML 1.0 Strict\" height=\"31\" width=\"88\" ");
  printf ("/></a></p>");


  printf ("</body></html>");

  if (file)
    {
      munmap (code_start, code_len);
      close (file);
    }

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

  printf ("<p>Available files in /brainfuck:</p>\n");

  printf ("<form action=\"brainfuck.cgi\" method=\"post\"><p>\n");
  while ((de = readdir (dp)))
    {
      if (de->d_type == DT_UNKNOWN)
        {
          dup_name = tmp = strdup (de->d_name);
          for (; *tmp != '.'; tmp++);
          *tmp = '\0';
          printf ("<label>%s<input type=\"radio\" name=\"file\" value=\"%s\"/>",
                  de->d_name, dup_name);
          printf ("</label><br />\n");
        }
    }

  printf ("<br />User supplied code (file selection takes precedence):");
  printf ("<br />\n<textarea name=\"code\" rows=\"10\" cols=\"50\">+[,.]");
  printf ("</textarea><br />\n");

  printf ("<br />User supplied input:<br />");
  printf ("<input type=\"text\" name=\"input\" /><br />\n");

  printf ("<input type=\"submit\" value=\"Send\" /><br />\n");
  printf ("<input type=\"reset\" value=\"Clear\" /><br />\n");

  printf ("</p></form>\n");
}


int
main (void)
{
  char *filename;

  s_cgi *cgi;

  struct stat sb;

  cgi = cgiInit ();

  cgiHeader ();

  if ((filename = cgiGetValue (cgi, "file")))
    header (filename);
  else if ((code = cgiGetValue (cgi, "code")))
    header ("User supplied code");
  else
    {
      header ("Brainfuck interpreter");
      main_site ();
      footer (EXIT_SUCCESS);
    }

  filename = (valid_filename (filename));

  if (filename)
    {
      if ((file = open (filename, O_RDONLY)) == -1)
        {
          printf ("<p>No such file: /%s</p>\n", filename+3);
          footer (EXIT_FAILURE);
        }
      if (fstat (file, &sb) == -1)
        {
          printf ("<p>Problem getting file lenght</p>\n");
          footer (EXIT_FAILURE);
        }

      code_len = sb.st_size;

      if ((code = mmap (NULL, code_len, PROT_READ, MAP_PRIVATE, file, 0))
          == MAP_FAILED)
        {
          printf ("<p>Problem memory mapping file</p>\n");
          footer (EXIT_FAILURE);
        }
    }
  else if (code)
    code_len = strlen (code);


  if (!code)
    {
      printf ("<p>No code to interpret\n</p>");
      footer (EXIT_FAILURE);
    }

  code_start = code;
  code_end = code + code_len;

  if (!(cells = malloc (30000)))
    {
      printf ("<p>NOT ENOUGH MEMORIES!</p>");
      footer (EXIT_FAILURE);
    }

  gettimeofday (&start_time, NULL);
  timeradd (&start_time, &offset, &stop_time);

  cell = 0;
  memset (cells, 0, 30000);

  if ((input = cgiGetValue (cgi, "input")))
    input_len = strlen (input);
  else
    input_len = 0;

  if (filename)
    printf ("\n\n<p><a href=\"/%s\">Source code</a>:</p>",
            filename+3);
  else
    printf ("\n\n</pre><p>Source code</a>:</p>");

  printf ("<pre>");
  code = code_start;
  while (code < code_end)
    printf ("%c", *code++);

  code = code_start;

  printf ("</pre><hr />");

  if (input)
    printf ("Input:<pre>%s</pre><hr />", input);


  i = 0;

  printf ("<p>Output:</p><pre>\n\n");
  interpret (0);

  printf ("</pre>\n\n");

  footer (EXIT_SUCCESS);

  return 0;
}
