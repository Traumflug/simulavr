/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA..
 *
 */

/*
 *  This port correponds to the "-W 0x52,-" command line option.
 */
#define special_output_port (*( (volatile char *)0x52))

/*
 *  This port correponds to the "-e 0x4F" command line option.
 */
#define special_exit_port (*( (volatile char *)0x4F))

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
 *  Trigger exit from simulation
 */
void sim_exit(int c)
{
  special_exit_port = (unsigned char)c;
}


int main()
{
  debug_puts(
    "\n"
    "This program tests the simulator magic exit port.\n"
    "There should be no more messages after this one.\n"
  );

  sim_exit(4);

  debug_puts( "ERROR - Simulation did not exit?\n" );

  return 0;
}

/* EOF */
