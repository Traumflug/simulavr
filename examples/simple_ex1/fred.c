/*
 *  This is a test program to demonstrate use of the debug input and output
 *  port.
 *
 *  $Id$
 */

/*
 *  This port correponds to the "-W 0x20,-" command line option.
 */
#define special_output_port (*( (volatile char *)0x20))

/*
 *  This port correponds to the "-R 0x22,-" command line option.
 */
#define special_input_port  (*( (volatile char *)0x22))

/*
 *  Poll the specified string out the debug port.
 */
void debug_puts(const char *str)
{
  const char *c;

  for ( c=str ; *c ; c++ )
    special_output_port = *c;
}

/*
 *  Main for test program.  Enter a string and echo it.
 */
int main()
{
  volatile char in_char;

  /*
   * Output the prompt string
   */
  debug_puts(
    "\n\n"
    "Press any key(and enter,if input is buffered, i.e. from keyboard):"
    "\n> "
  );

  /*
   * Input one character but since line buffered, blocks until a CR.
   */
  in_char = special_input_port;

  /*
   *  Print the "what you entered:" message.
   */
  debug_puts( "\nYou entered: " );

  /*
   * now echo the rest of the characters
   */
  do {
    special_output_port = in_char;
  } while ( (in_char = special_input_port) != '\n' );

  special_output_port = '\n';
  special_output_port = '\n';

  return 0;
}
