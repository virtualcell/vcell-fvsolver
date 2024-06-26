
# Version -- g(eneric), v(ector), c(ray)

VERSION = g

# Fortran Options

# Standard Fortran options
FOPT     = -fast
# Special Fortran options
SFOPT    = 
FFLAGS   = ${FOPT} ${SFOPT}

# Source Code Directories

# Directories for routines that are the same in all versions
# SDIR contains routines without external statements
GDIR     = .
# SEDIR contains routines with external statements
GEDIR     = .

# Directories for routines that change in all vector versions, but
# do not have Cray-specific versions
# VDIR contains routines without external statements
VDIR     = .
# VDIR contains routines with external statements
VEDIR    = .

# Directories for routines that change in all vector versions, and
# do have Cray-specific versions
# VCDIR contains routines without external statements
VCDIR     = .
# VCEDIR contains routines with external statements
VCEDIR    = .

# Directories for routines that change only in Cray versions
# CDIR contains routines without external statements
CDIR     = .
# CEDIR contains routines with external statements
CEDIR    = .

# Library Name

# LIBDIR contains PCGPAK2 library
LIBDIR   = ../../lib
# Precision options -- s(ingle), d(ouble), c(complex)
PRECOPT  = d
# Debug part of library name
DB       =
# Special options part of library name
LBOPT    =
# LIBRARY name is {s,d,c}{blank,v,c}pcglib{dbg option}{spec options}.
LIBRARY  = ${LIBDIR}/${VERSION}${PRECOPT}pcglib${DB}${LBOPT}.a

# Source File Names

GNOEXTS  =  pc0v.o    pcijup.o  pcmlic.o  pcrdpt.o  pcsrfp.o \
	    pc0vi.o   pcilu.o   pcmlir.o  pcric.o   pcsrij.o \
	    pcaxpy.o  pcilu1.o  pcmlr.o   pcrilu.o  pcsrix.o \
	    pcbtrc.o  pciluf.o  pcmuc.o   pcrsad.o  pcsriz.o \
	    pccred.o  pcilus.o  pcmuic.o  pcrsbs.o  pcsro2.o \
	    pcd2i.o   pciluz.o  pcmuir.o  pcrsfb.o  pcsrob.o \
	    pcd2ii.o  pciprm.o  pcmur.o   pcrsfm.o  pcsrop.o \
	    pcd2it.o  pciprp.o  pcmvc.o   pcrsfp.o  pcsrrb.o \
	    pcdmv.o   pcirsw.o  pcmvr.o   pcrslu.o  pcsrrp.o \
	    pcdmva.o  pclu1c.o  pcmvs.o   pcrspf.o  pcsrsb.o \
	    pciabs.o  pclu1r.o  pcnegv.o  pcrsps.o  pcsrsp.o \
	    pciblk.o  pclub1.o  pcnopr.o  pcrsrb.o  pcsrti.o \
	    pcic.o    pclub2.o  pcpack.o  pcrsrp.o  pcsrzp.o \
	    pcic1.o   pclub3.o  pcpaxp.o  pcsbge.o  pcsscl.o \
	    pcicb.o   pclub4.o  pcpcnf.o  pcsbgp.o  pctran.o \
	    pcicmc.o  pclub5.o  pcpcsf.o  pcsc1b.o  pcunpk.o \
	    pcicmr.o  pclubf.o  pcprmb.o  pcsc1p.o  pcvadd.o \
	    pcicms.o  pclup.o   pcprmp.o  pcsc2b.o  pcvdiv.o \
	    pcicpr.o  pclup1.o  pcpunp.o  pcscal.o  pcverf.o \
	    pcicpy.o  pclup2.o  pcrc1b.o  pcsd2f.o  pcvers.o \
	    pcictr.o  pclup3.o  pcrc1p.o  pcsd2s.o  pcvmul.o \
	    pcictx.o  pclup4.o  pcrc2b.o  pcsijb.o  pcvsma.o \
	    pcicx.o   pclup5.o  pcrcpr.o  pcspfa.o  pcvsub.o \
	    pciipr.o  pclupf.o  pcrcpy.o  pcspsl.o  pcvtm.o  \
	    pcijab.o  pcmlc.o   pcrdpp.o  pcsrfb.o pcdslv.o

GEXTS    =  pccg.o    pcgcr.o   pcop.o    pcsp.o   \
	    pccgx.o   pcgmr.o   pcredb.o  pcspr.o  \
	    pcdrv.o   pcity.o   pcredp.o  pcsrb.o  \
	    pcend.o   pcomn.o   pcredu.o  pcsrp.o

VNOEXTS  =  pcily.o   pclub.o   pcmv.o    pcvunp.o

VEXTS    =  pcdrvx.o

VCEXTS   =  pcgmrx.o

CEXTS    =  pcitx.o

pcgpak2:    ${LIBRARY}
${LIBRARY}: ${GNOEXTS} ${GEXTS} ${VNOEXTS} ${VEXTS} ${VCEXTS} ${CEXTS}
	  ar rv ${LIBRARY} *.o
	  rm *.o
	  ranlib ${LIBRARY}
${GNOEXTS}:
	  f77 $(FFLAGS) -c $(GDIR)/$*.f
${GEXTS}:
	  f77 $(FFLAGS) -c $(GEDIR)/$*.f
${VNOEXTS}:
	  f77 $(FFLAGS) -c $(VDIR)/$*.f
${VEXTS}:
	  f77 $(FFLAGS) -c $(VEDIR)/$*.f
${VCEXTS}:
	  f77 $(FFLAGS) -c $(VCEDIR)/$*.f
${CEXTS}:
	  f77 $(FFLAGS) -c $(CEDIR)/$*.f
