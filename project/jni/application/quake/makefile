#
# DINGUX SDK for Cygwin / Version: 0.1
# DINGUX Makefile by Rikku2000
#

#
#========================================(Config)
#

# Software Name
PROGRAM = quake_r2-tec.dge

# Phat
DINGUXDIR := /dingux
DINGUXBIN := $(DINGUXDIR)/bin
DINGUXLIB := $(DINGUXDIR)/mipsel-linux-uclibc/lib
DINGUXINC := $(DINGUXDIR)/mipsel-linux-uclibc/include

# Compiler Name
ARCH := mipsel-linux-uclibc-

# Compiler
CC := ${DINGUXBIN}/$(ARCH)gcc
CXX := ${DINGUXBIN}/$(ARCH)g++
STRIP := ${DINGUXBIN}/$(ARCH)strip

# Linker
LDFLAGS = $(shell $(DINGUXBIN)/sdl-config --libs) -lSDL -lSDL_gfx -lSDL_ttf -lSDL_image -lSDL_mixer -ljpeg -lpng12 -lz -lm
CFLAGS = $(shell $(DINGUXBIN)/sdl-config --cflags) -Wall -pipe -c -O2 -msoft-float -ffast-math -funroll-loops -falign-loops=2 -falign-jumps=2 -falign-functions=2 -fno-strict-aliasing
CXXFLAGS = $(shell $(DINGUXBIN)/sdl-config --cflags)

# Include
INCLUDES := -I${DINGUXINC} -I${DINGUXINC}/SDL

# Libs
LIBS += $(shell $(DINGUXBIN)/sdl-config --libs)

#
#========================================(Files)
#

CFILES	=			source/cd_null.c \
					source/cpu.c \
					source/vid_sdl.c \
					source/snd_sdl.c \
					source/sys_sdl.c \
					source/chase.c \
					source/cl_demo.c \
					source/cl_input.c \
					source/cl_main.c \
					source/cl_parse.c \
					source/cl_tent.c \
					source/cmd.c \
					source/common.c \
					source/console.c \
					source/crc.c \
					source/cvar.c \
					source/d_edge.c \
					source/d_fill.c \
					source/d_init.c \
					source/d_modech.c \
					source/d_part.c \
					source/d_polyse.c \
					source/d_scan.c \
					source/d_sky.c \
					source/d_sprite.c \
					source/d_surf.c \
					source/d_vars.c \
					source/d_zpoint.c \
					source/draw.c \
					source/FixedPointMath.c \
					source/host.c \
					source/host_cmd.c \
					source/keys.c \
					source/mathlib.c \
					source/menu.c \
					source/model.c \
					source/net_dgrm.c \
					source/net_loop.c \
					source/net_main.c \
					source/net_vcr.c \
					source/net_udp.c \
					source/net_bsd.c \
					source/nonintel.c \
					source/pr_cmds.c \
					source/pr_edict.c \
					source/pr_exec.c \
					source/r_aclip.c \
					source/r_alias.c \
					source/r_bsp.c \
					source/r_draw.c \
					source/r_edge.c \
					source/r_efrag.c \
					source/r_light.c \
					source/r_main.c \
					source/r_misc.c \
					source/r_part.c \
					source/r_sky.c \
					source/r_sprite.c \
					source/r_surf.c \
					source/r_vars.c \
					source/sbar.c \
					source/screen.c \
					source/snd_dma.c \
					source/snd_mix.c \
					source/slist.c \
					source/sv_main.c \
					source/sv_move.c \
					source/sv_phys.c \
					source/sv_user.c \
					source/snd_mem.c \
					source/view.c \
					source/wad.c \
					source/world.c \
					source/zone.c

#
#========================================(Compile)
#

OFILES = $(SFILES:.S=.o) $(CFILES:.c=.o)

$(PROGRAM):	$(OFILES)
			$(CXX) $(OFILES) $(LDFLAGS) -o $@
			$(STRIP) $(PROGRAM)

all: $(PROGRAM)

%.o: %.c
	 $(CC) $(ALL_INCLUDES) $(CFLAGS) -c $< -o $@

clean:
	 -rm -f $(OFILES) $(MAPFILE) $(PROGRAM)
