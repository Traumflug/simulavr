ATTENTION:

If you give in addition to the debug option (-g) 
the -n (no wait for gdb connection) you MUST give
a elf file with -f option, otherwice you will 
crash the simulator.
    

Remember: If you also want to activate the -t/--trace
option you have also give a file for the symbols. gdb is
not able to give the symbol names over the gdb port interface.



