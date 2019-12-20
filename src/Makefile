########################################################################
# Copyright (c) 2019, Gil Treibush
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2, as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# A copy of the full GNU General Public License is included in this
# distribution in a file called "COPYING" or "LICENSE".
########################################################################

OUT_FILE = gilcc

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(OUT_FILE)

$(OUT_FILE): $(OBJS)
	$(CC) -o $(@) $(^) $(LFLAGS)

$(OBJS): $(SRCS)

clean:
	@rm -fr $(OUT_FILE) *.o

