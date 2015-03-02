#
# Makefile for stoopy
#
# Copyright (C) 2005-2011, Kory Hamzeh, Avatar Consultants, Inc.
# This program is licensed under the GNU General Public License V3.
#
#
VERSION=1.0
#
SRC = stoopy.c

TARFILES = Makefile $(SRC)

stoopy: stoopy.c
	gcc -g -Bstatic -o stoopy stoopy.c

clean:
	rm -f stoopy stoopy.o

tgz:
	tar cvzf stoopy-src-$(VERSION).tgz $(TARFILES)

