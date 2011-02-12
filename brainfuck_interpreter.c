#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include <ctype.h>

#include <errno.h>


static char *input;
static int input_len;

static int i = 0;

static unsigned char *cells;
static unsigned int cell;

static char *code = 0;
static char *code_start;
static char *code_end;
static int code_len;


static struct timeval stop_time;
static struct timeval start_time;
static struct timeval offset = {5, 0};


void
default_print_cell (char c)
{
  printf ("%c", c);
}

static void (*print_cell) (char)  = default_print_cell;

int
init_interpreter (int c_len, char *c, int i_len, char *i, void (*cb) (char))
{
  if (!c_len && !c)
    {
      errno = EIO;
      return -1;
    }
  code_len = c_len;
  code = c;

  code_start = code;
  code_end = code + code_len;


  input_len = i_len;
  input = i;


  if (!(cells = malloc (30000)))
    {
      errno = ENOMEM;
      return -1;
    }
  cell = 0;
  memset (cells, 0, 30000);


  gettimeofday (&start_time, NULL);
  timeradd (&start_time, &offset, &stop_time);

  if (cb)
    print_cell = cb;

  return 0;
}

int
interpret (int ignore)
{
  int code_pos = i;
  char instruction;


  for (; code < code_end; i++)
    {

      gettimeofday (&start_time, NULL);

      if (timercmp (&start_time, &stop_time, >))
        {
          errno = ETIME;
          return -1;
        }

      instruction = *(code++);

      if (ignore)
        {
          if (instruction == '[')
            {
              i++;
              if (interpret (ignore) == -1)
                return -1;
              continue;
            }
          else if (instruction == ']')
            {
              return 0;
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
          print_cell (cells[cell]);
          break;
        case ',':
          if (input_len--)
            cells[cell] = *(input++);
          else
            {
              errno = EIO;
              return -1;
            }
          break;
        case '[':
          if (cells[cell])
            {
              i++;
              if (interpret (0) == -1)
                return -1;
            }
          else
            {
              i++;
              if (interpret (1) == -1)
                return -1;
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
              return 0;
            }
          break;
        }
    }
}
