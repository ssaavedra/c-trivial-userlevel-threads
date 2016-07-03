# c-trivial-userlevel-threads
UserLevel co-routines implemented in C for IA-32 and x86-64.

This started as an assignment for a Design of Operating Systems class, in the University of A Coruña.

Compatibility
=============

This tool is written in non-portable C, and is expected to run on IA-32 processors successfully, and with a few caveats on x86-64 (currently x86-64 does not support preemption, because I may have messed up with the %sp and %bp locations).


Author
======

Santiago Saavedra


License
=======

The code is hereby released under the a MIT License. Experiment with it.

