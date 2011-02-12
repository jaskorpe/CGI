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


#include <fcntl.h>

#include <unistd.h>

#include <cgi.h>

#include <errno.h>

#include <sys/mman.h>

#include "brainfuck_interpreter.h"

static void header (char *title);
static void footer (int exit_status);
static char *valid_filename (char *name);
static void main_site (void);



static int file;

static char *input;
static int input_len;


static char *tmp;

static int code_len = 0;
static char *code = 0;
static char *code_start, *code_end;

void
print_cell (char c)
{
  switch (c)
    {
    case '<':
      printf ("&lt;");
      break;
    case '&':
      printf ("&amp;");
      break;
    default:
      printf ("%c", c);
      break;
    }
}

void
header (char *title)
{
  printf ("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n"
          "\t\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
          "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");

  if (title)
    printf ("<head><title>%s</title>\n", title);
  else
    printf ("<head><title>Brainfuck interpreter</title>\n");

  printf ("<meta http-equiv=\"content-type\"\n"
          "\tcontent=\"text/html;charset=utf-8\" />\n"
          "<script type=\"text/javascript\" src=\"/functions.js\"></script>\n"
          "</head><body>\n");
}


void
footer (int exit_status)
{
  printf ("<hr /><p>Brainfuck interpreter completely written in C</p>\n"
          "<p>Copyright (C) 2010 Jon Anders Skorpen</p><hr />\n"

          "<p>Get source code <a href=\"http://github.com/jaskorpe/CGI\">\n"
          "here</a>.</p><hr /><p>Version: 1.1</p><hr />\n"

          "<p><a href=\"http://validator.w3.org/check?uri=referer\">\n"
          "<img src=\"http://www.w3.org/Icons/valid-xhtml10\"\n"
          "\talt=\"Valid XHTML 1.0 Strict\" height=\"31\" width=\"88\""
          " /></a></p>"

          "</body></html>");

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

  printf ("<p>Welcome to my Brainfuck online interpreter.</p>");

  if ((dp = opendir ("../brainfuck")))
    {
      printf ("<p>Available files in /brainfuck:<br />\n");
      while ((de = readdir (dp)))
        {
          if (de->d_type == DT_REG)
            {
              printf ("<a href=\"/brainfuck/%s\" "
                      "onclick=\"loadBf('%s'); return false;\">"
                      "%s</a><br />\n",
                      de->d_name, de->d_name, de->d_name);
            }
        }
    }

  printf ("</p><form action=\"brainfuck.cgi\" id=\"bfForm\" "
          "method=\"post\"><p>\n"
          "<br />\n<textarea name=\"code\" id=\"bfcode\" "
          "rows=\"25\" cols=\"60\">+[,.]"
          "</textarea><br />\n<br />User supplied input:<br />"
          "<input type=\"text\" name=\"input\" /><br />\n"
          "<input type=\"submit\" value=\"Send\" /><br />\n"
          "<input type=\"reset\" value=\"Clear\" /><br />\n"

          "</p></form>\n");
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

  code_start = code;
  code_end = code + code_len;

  if ((input = cgiGetValue (cgi, "input")))
    input_len = strlen (input);
  else
    input_len = 0;


  if (init_interpreter (code_len, code, input_len, input, print_cell) == -1)
    {
      if (errno == ENOMEM)
        printf ("<p>NOT ENOUGH MEMORIES!</p>");
      else if (errno == EIO)
        printf ("<p>No code to interpret\n</p>");
      else
        printf ("<p>Some undefined error occured\n</p>");

      footer (EXIT_FAILURE);
    }



  if (filename)
    printf ("\n\n<p><a href=\"/%s\">Source code</a>:</p>",
            filename+3);
  else
    printf ("\n\n<p>Source code</a>:</p>");

  printf ("<pre>");

  code_start = code;
  code_end = code + code_len;
  while (code < code_end)
    printf ("%c", *code++);

  printf ("</pre><hr />");

  tmp = input;
  while (tmp && *tmp)
    print_cell (*tmp++);

  printf ("</pre><hr />");


  printf ("<p>Output:</p><pre>\n\n");

  if (interpret (0) == -1)
    {
      if (errno == ETIME)
        printf ("</pre>\n<p>Timeout!</p>");
      else if (errno == EIO)
        printf ("\n\n</pre>\n<hr /><p>Trouble reading from input</p>");
      else
        printf ("\n</pre>\n<hr /><p>Undefined error</p>");
      footer (EXIT_FAILURE);
    }
  printf ("</pre>\n\n");

  footer (EXIT_SUCCESS);

  return 0;
}
