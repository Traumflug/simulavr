ATTENTION:

In actual version you MUST give a avr elf-file for simulavrxx also in
debugger (gdb) mode. Execution of elf file will start BEFORE the gdb
connection is ready. That is not a bug, that is a feature :-)

Why that solution:
If you connect multiple gdb´s to the simulator (if you
have multiple cores in one simulation) the simulator
must handle multiple gdb ports. If none of the gdb´s give a 
BREAK to simulavrxx the simulator will continue execution.
Before we have a connection we will have no break so the execution
starts directly whithout gdb and this means without code if
you have not given a file on commandline or in tcl script or
in your special c/c++ start file. To make a default break
is not a solution because you could also start with many
gdb ports but connect only to one or more of them. If
we will wait for all ports with a continue you must
use all gdb connections with one running gdb for all ports what
is more inconvinient as giving a default file on command line.

You are able to reset a core after you have connected to
simulavrxx! Simply give the load command to gdb. So you
can debug your program from beginning.

In future versions there will be a check for
"no code" and "no gdb" which stops execution.

Remember: If you also want to activate the -t/--trace
option you have also give a file for the symbols. gdb is
not able to give the symbol names over the gdb port interface.


