/*
 *  $Id$
 */

#ifndef _avrtest_help_h
#define _avrtest_help_h

/*
 *  Poll the specified string out the debug port.
 */
void debug_puts(const char *str);

void sim_abort(void);

void sim_exit(int);

#endif
