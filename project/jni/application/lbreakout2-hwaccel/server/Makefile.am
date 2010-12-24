DEFS = @DEFS@ @network_flag@ @sdlnet_flag@ @inst_flag@ @locdir_flag@

bin_PROGRAMS = lbreakout2server

lbreakout2server_SOURCES = \
  server.c server_game.c

EXTRA_DIST = server.h server_game.h

lbreakout2server_LDADD = ../common/libcommon.a ../game/libgame.a @sdlnet_lib_flag@

SUBDIRS = levels
