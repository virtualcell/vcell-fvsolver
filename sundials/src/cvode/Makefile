# -----------------------------------------------------------------
# $Revision: 1.4 $
# $Date: 2006/08/11 23:03:04 $
# -----------------------------------------------------------------
# Programmer(s): Radu Serban and Aaron Collier @ LLNL
# -----------------------------------------------------------------
# Copyright (c) 2002, The Regents of the University of California.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# -----------------------------------------------------------------
# Makefile for CVODE module
#
# src/cvode/Makefile.  Generated from Makefile.in by configure.
# -----------------------------------------------------------------

SHELL = /bin/sh



srcdir       = .
builddir     = .
abs_builddir = /home/VCELL/fgao/winhome/sundials-2.3.0/src/cvode
top_builddir = ../..
prefix       = /usr/local
exec_prefix  = ${prefix}
includedir   = ${prefix}/include
libdir       = ${exec_prefix}/lib

INSTALL        = /usr/bin/install -c
INSTALL_LIB    = ${INSTALL}
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

LIB_REVISION = 0:1:0

CVODE_LIB       = libsundials_cvode.la
CVODE_LIB_FILES = cvode.lo cvode_io.lo cvode_band.lo cvode_dense.lo cvode_diag.lo cvode_spils.lo cvode_spbcgs.lo cvode_spgmr.lo cvode_sptfqmr.lo cvode_bandpre.lo cvode_bbdpre.lo
CVODE_OBJ_FILES = cvode.o cvode_io.o cvode_band.o cvode_dense.o cvode_diag.o cvode_spils.o cvode_spbcgs.o cvode_spgmr.o cvode_sptfqmr.o cvode_bandpre.o cvode_bbdpre.o

SHARED_LIB_FILES = $(top_builddir)/src/sundials/sundials_band.lo \
                   $(top_builddir)/src/sundials/sundials_dense.lo \
                   $(top_builddir)/src/sundials/sundials_iterative.lo \
                   $(top_builddir)/src/sundials/sundials_math.lo \
                   $(top_builddir)/src/sundials/sundials_nvector.lo \
                   $(top_builddir)/src/sundials/sundials_smalldense.lo \
                   $(top_builddir)/src/sundials/sundials_spgmr.lo \
                   $(top_builddir)/src/sundials/sundials_spbcgs.lo \
                   $(top_builddir)/src/sundials/sundials_sptfqmr.lo

mkinstalldirs = $(SHELL) $(top_srcdir)/config/mkinstalldirs

all: $(CVODE_LIB)

$(CVODE_LIB): shared $(CVODE_LIB_FILES)
	$(LIBTOOL) --mode=link $(CC) $(CFLAGS) -o $(CVODE_LIB) $(CVODE_LIB_FILES) $(SHARED_LIB_FILES) -rpath $(libdir) $(LDFLAGS) $(LIBS) -version-info $(LIB_REVISION)

install: $(CVODE_LIB)
	$(mkinstalldirs) $(includedir)/cvode
	$(mkinstalldirs) $(libdir)
	$(LIBTOOL) --mode=install $(INSTALL_LIB) $(CVODE_LIB) $(libdir)
	$(INSTALL_HEADER) $(top_srcdir)/include/cvode/cvode.h         $(includedir)/cvode/
	$(INSTALL_HEADER) $(top_srcdir)/include/cvode/cvode_dense.h   $(includedir)/cvode/
	$(INSTALL_HEADER) $(top_srcdir)/include/cvode/cvode_band.h    $(includedir)/cvode/
	$(INSTALL_HEADER) $(top_srcdir)/include/cvode/cvode_diag.h    $(includedir)/cvode/
	$(INSTALL_HEADER) $(top_srcdir)/include/cvode/cvode_spils.h   $(includedir)/cvode/
	$(INSTALL_HEADER) $(top_srcdir)/include/cvode/cvode_spbcgs.h  $(includedir)/cvode/
	$(INSTALL_HEADER) $(top_srcdir)/include/cvode/cvode_spgmr.h   $(includedir)/cvode/
	$(INSTALL_HEADER) $(top_srcdir)/include/cvode/cvode_sptfqmr.h $(includedir)/cvode/
	$(INSTALL_HEADER) $(top_srcdir)/include/cvode/cvode_bandpre.h $(includedir)/cvode/
	$(INSTALL_HEADER) $(top_srcdir)/include/cvode/cvode_bbdpre.h  $(includedir)/cvode/
	$(INSTALL_HEADER) $(top_srcdir)/src/cvode/cvode_impl.h        $(includedir)/cvode/

uninstall:
	$(LIBTOOL) --mode=uninstall rm -f $(libdir)/$(CVODE_LIB)
	rm -f $(includedir)/cvode/cvode.h
	rm -f $(includedir)/cvode/cvode_dense.h
	rm -f $(includedir)/cvode/cvode_band.h
	rm -f $(includedir)/cvode/cvode_diag.h
	rm -f $(includedir)/cvode/cvode_spils.h
	rm -f $(includedir)/cvode/cvode_spbcgs.h
	rm -f $(includedir)/cvode/cvode_spgmr.h
	rm -f $(includedir)/cvode/cvode_sptfqmr.h
	rm -f $(includedir)/cvode/cvode_bandpre.h
	rm -f $(includedir)/cvode/cvode_bbdpre.h
	rm -f $(includedir)/cvode/cvode_impl.h
	@if test -d ${includedir}/cvode ; then \
	   rmdir ${includedir}/cvode ;         \
	 fi

shared:
	@cd ${top_builddir}/src/sundials ; \
	 ${MAKE} ;                         \
	 cd ${abs_builddir}

clean:
	$(LIBTOOL) --mode=clean rm -f $(CVODE_LIB)
	rm -f $(CVODE_LIB_FILES)
	rm -f $(CVODE_OBJ_FILES)

distclean: clean
	rm -f Makefile

cvode.lo: $(srcdir)/cvode.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/cvode.c
cvode_io.lo: $(srcdir)/cvode_io.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/cvode_io.c
cvode_dense.lo: $(srcdir)/cvode_dense.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/cvode_dense.c
cvode_band.lo: $(srcdir)/cvode_band.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/cvode_band.c
cvode_diag.lo: $(srcdir)/cvode_diag.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/cvode_diag.c
cvode_spils.lo: $(srcdir)/cvode_spils.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/cvode_spils.c
cvode_spbcgs.lo: $(srcdir)/cvode_spbcgs.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/cvode_spbcgs.c
cvode_spgmr.lo: $(srcdir)/cvode_spgmr.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/cvode_spgmr.c
cvode_sptfqmr.lo: $(srcdir)/cvode_sptfqmr.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/cvode_sptfqmr.c
cvode_bandpre.lo: $(srcdir)/cvode_bandpre.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/cvode_bandpre.c
cvode_bbdpre.lo: $(srcdir)/cvode_bbdpre.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/cvode_bbdpre.c

libtool: $(top_builddir)/$(LIBTOOL_DEPS)
	@cd ${top_builddir} ;                 \
	 ${SHELL} ./config.status --recheck ; \
	 cd ${abs_builddir}
