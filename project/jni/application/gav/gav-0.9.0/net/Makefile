# GAV - Gpl Arcade Volleyball
# Copyright (C) 2002
# GAV team (http://sourceforge.net/projects/gav/)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

RELPATH    = net
include ../CommonHeader

NAME 		= net
MODULE_NAME	= $(NAME)_module.o

CXXFLAGS += -I$(GAV_ROOT) -I$(GAV_ROOT)/menu -I$(GAV_ROOT)/net

DEPEND = Makefile.depend

.PHONY: all

all: $(MODULE_NAME)

$(MODULE_NAME): $(OFILES)
	$(LD) -r $(OFILES) -o $(MODULE_NAME)

($OFILES): $(SRCS)
	$(CXX) -c $(CXXFLAGS) $<

clean:
	rm -f *.o *.bin *~ $(DEPEND)

depend:
	$(RM) $(DEPEND)
	$(CXX) $(CXXFLAGS) -M $(SRCS) >> $(DEPEND)

ifeq ($(wildcard $(DEPEND)),$(DEPEND))
include $(DEPEND)
endif
