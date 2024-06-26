# -----------------------------------------------------------------
# $Revision: 1.5 $
# $Date: 2006/10/11 16:34:20 $
# -----------------------------------------------------------------
# Programmer(s): Radu Serban and Aaron Collier @ LLNL
# -----------------------------------------------------------------
# Copyright (c) 2002, The Regents of the University of California.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# -----------------------------------------------------------------
# Makefile for SHARED module
#
# src/sundials/Makefile.  Generated from Makefile.in by configure.
# -----------------------------------------------------------------

SHELL = /bin/sh

srcdir       = .
builddir     = .
abs_builddir = /home/VCELL/fgao/winhome/sundials-2.3.0/src/sundials
top_builddir = ../..
prefix       = /usr/local
exec_prefix  = ${prefix}
includedir   = ${prefix}/include
libdir       = ${exec_prefix}/lib
bindir       = ${exec_prefix}/bin

INSTALL        = /usr/bin/install -c
INSTALL_PROG   = ${INSTALL}
INSTALL_HEADER = ${INSTALL} -m 644

LIBTOOL      = $(SHELL) $(top_builddir)/libtool
LIBTOOL_DEPS = config/ltmain.sh

CPP      = cc -E
CPPFLAGS = 
CC       = cc
CFLAGS   = 
LDFLAGS  = 
LIBS     = -lm 

top_srcdir = $(srcdir)/../..

INCLUDES = -I$(top_builddir)/src -I$(top_srcdir)/include

SHARED_LIB_FILES = sundials_band.lo sundials_dense.lo sundials_iterative.lo sundials_math.lo sundials_nvector.lo sundials_smalldense.lo sundials_spgmr.lo sundials_spbcgs.lo sundials_sptfqmr.lo
SHARED_OBJ_FILES = sundials_band.o sundials_dense.o sundials_iterative.o sundials_math.o sundials_nvector.o sundials_smalldense.o sundials_spgmr.o sundials_spbcgs.o sundials_sptfqmr.o

mkinstalldirs = $(SHELL) $(top_srcdir)/config/mkinstalldirs

all: $(SHARED_LIB_FILES)

install: $(SHARED_LIB_FILES)
	$(mkinstalldirs)  $(includedir)/sundials
	$(mkinstalldirs)  $(bindir)
	$(INSTALL_HEADER) $(top_srcdir)/include/sundials/sundials_band.h       $(includedir)/sundials/
	$(INSTALL_HEADER) $(top_srcdir)/include/sundials/sundials_dense.h      $(includedir)/sundials/
	$(INSTALL_HEADER) $(top_srcdir)/include/sundials/sundials_iterative.h  $(includedir)/sundials/
	$(INSTALL_HEADER) $(top_srcdir)/include/sundials/sundials_math.h       $(includedir)/sundials/
	$(INSTALL_HEADER) $(top_srcdir)/include/sundials/sundials_types.h      $(includedir)/sundials/
	$(INSTALL_HEADER) $(top_srcdir)/include/sundials/sundials_nvector.h    $(includedir)/sundials/
	$(INSTALL_HEADER) $(top_srcdir)/include/sundials/sundials_smalldense.h $(includedir)/sundials/
	$(INSTALL_HEADER) $(top_srcdir)/include/sundials/sundials_spgmr.h      $(includedir)/sundials/
	$(INSTALL_HEADER) $(top_srcdir)/include/sundials/sundials_spbcgs.h     $(includedir)/sundials/
	$(INSTALL_HEADER) $(top_srcdir)/include/sundials/sundials_sptfqmr.h    $(includedir)/sundials/
	$(INSTALL_HEADER) $(top_builddir)/src/sundials/sundials_config.h       $(includedir)/sundials/
	$(INSTALL_PROG)   $(top_builddir)/sundials-config                      $(bindir)/

uninstall:
	rm -f $(includedir)/sundials/sundials_band.h
	rm -f $(includedir)/sundials/sundials_dense.h
	rm -f $(includedir)/sundials/sundials_iterative.h
	rm -f $(includedir)/sundials/sundials_math.h
	rm -f $(includedir)/sundials/sundials_types.h
	rm -f $(includedir)/sundials/sundials_nvector.h
	rm -f $(includedir)/sundials/sundials_smalldense.h
	rm -f $(includedir)/sundials/sundials_spgmr.h
	rm -f $(includedir)/sundials/sundials_spbcgs.h
	rm -f $(includedir)/sundials/sundials_sptfqmr.h
	rm -f $(includedir)/sundials/sundials_config.h
	@if test -d ${includedir}/sundials ; then \
	   rmdir ${includedir}/sundials ;         \
	 fi
	rm -f $(bindir)/sundials-config

clean:
	rm -f $(SHARED_LIB_FILES)
	rm -f $(SHARED_OBJ_FILES)
	rm -rf .libs

distclean: clean
	rm -f Makefile

sundials_band.lo: $(srcdir)/sundials_band.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/sundials_band.c
sundials_dense.lo: $(srcdir)/sundials_dense.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/sundials_dense.c
sundials_iterative.lo: $(srcdir)/sundials_iterative.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/sundials_iterative.c
sundials_math.lo: $(srcdir)/sundials_math.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/sundials_math.c
sundials_nvector.lo: $(srcdir)/sundials_nvector.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/sundials_nvector.c
sundials_smalldense.lo: $(srcdir)/sundials_smalldense.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/sundials_smalldense.c
sundials_spgmr.lo: $(srcdir)/sundials_spgmr.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/sundials_spgmr.c
sundials_spbcgs.lo: $(srcdir)/sundials_spbcgs.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/sundials_spbcgs.c
sundials_sptfqmr.lo: $(srcdir)/sundials_sptfqmr.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/sundials_sptfqmr.c

libtool: $(top_builddir)/$(LIBTOOL_DEPS)
	@cd ${top_builddir} ;                 \
	 ${SHELL} ./config.status --recheck ; \
	 cd ${abs_builddir}
