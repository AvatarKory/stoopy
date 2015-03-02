
Really Stupid and Dumb Terminal Program
=======================================

Copyright (C) 2005-2015, Kory Hamzeh, Avatar Consultants, Inc.
This program is licensed under the GNU General Public License V2.

Release Notes V1.01
-------------------

The following changes were made in this release:
* The "slow write" option is disabled by default. Using the -s option
  enables it.
* Added the parity options (-e and -o).
* Added the hardware flow control (-f) option.
* Minor code clean up.

Release Notes V1.0
------------------

Stoopy is a very lightweight dumb terminal application that can
be used to talk to another device over the serial port. I wrote
this because I needed something small and simple without the 
overhead of minicom.

Stoopy is really dumb. If you hit Control-D at any time, it
will exit. By default, characters typed by the user and
sent to the serial device have an inter-character delay
of 50 milliseconds. This can be changed by the -d option
or totally disabled with the -s option. NOTE: the
slow write feature is disabled by default in later
releases.

Note that I have only tested this on Federoa Core and CentOS
Linux distributions, but the termio library used by stoopy
is POSIX compliant so it should port to other OS'es fairly
easily.

To build it, just type 'make' at the shell prompt.

Note that most serial tty devices are owned by root and 
permissions are set to 600. So you might have to run
stoopy as root, or set the sticky bit, or change the
permission of the device to 666 (but you need to be root
to do that).

Release 1.0 does not support 7 data bit, odd or even parity,
or hardware flow control. One day I will add those features.
If you need those option now, please contact me at kory
at avatar dot com and I will try to add it.

Have fun.

TO DO
-----

* Need to create a serial device lock file. Currently,
  it is possible to run two copies and stoopy using the
  same serial port device and that results is Bad
  Things Happening.

