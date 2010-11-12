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

include CommonHeader

ROOT=

SUBDIRS = menu automa net
CXXFLAGS += $(foreach DIR, $(SUBDIRS), -I$(DIR)) -I.

ALL_OBJ  = $(foreach DIR, $(SUBDIRS), $(DIR)/$(DIR:%=%_module.o))

DEPEND = Makefile.depend

.PHONY: depend clean all $(SUBDIRS)

all: $(SUBDIRS) gav

$(SUBDIRS):
	$(MAKE) -C $@

$(ALL_OBJ):
	$(MAKE) -C $(@D:%_module.o=%)

gav:	$(ALL_OBJ) $(OFILES)
	$(CXX) -o gav $(OFILES) $(ALL_OBJ) $(LDFLAGS)
	strip gav

clean:
	for i in $(SUBDIRS) ; do \
	  $(MAKE) -C $$i clean;\
	done
	rm -f *~ *.o gav $(DEPEND)

bindist: all
	for i in $(SUBDIRS) ; do \
	  $(MAKE) -C $$i clean;\
	done
	rm -f *~ *.o

install: all
	install gav $(ROOT)/usr/bin
	install -d $(ROOT)/usr/share/games/gav/themes
	cp -r themes/* $(ROOT)/usr/share/games/gav/themes

depend:
	for i in $(SUBDIRS) ; do \
	  $(MAKE) -C $$i depend;\
	done
	$(RM) $(DEPEND)
	$(CXX) -M $(CXXFLAGS) $(SRCS) >> $(DEPEND)

ifeq ($(wildcard $(DEPEND)),$(DEPEND))
include $(DEPEND)
endif
