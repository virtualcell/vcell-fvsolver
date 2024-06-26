      SUBROUTINEPCDRV(N,IJA,A,RHS,X,IPARM,RPARM,NSP,ISP,RSP,METHOD
     *,PRECON,PCSTOP,PCMV)
      IMPLICITDOUBLEPRECISION(A-H,O-Z)
      INTEGERN,IJA(*),IPARM(*),NSP,ISP(*)
      DOUBLEPRECISIONA(*),RHS(*),X(*),RPARM(*),RSP(*)
      EXTERNALMETHOD,PRECON,PCSTOP,PCMV
      INTEGERIERR,ITMAX,ITEST,KMAX,IFL,IPRE,PBLK,LUDONE,IBLK,
     *MNEED,ITS,RSDONE,NOVERF,RED1,RED2,IERR2,IFRMAT
      COMMON/PCCOM1/IERR,ITMAX,ITEST,KMAX,IFL,IPRE,PBLK,
     *LUDONE,IBLK,MNEED,ITS,RSDONE,NOVERF,RED1,RED2,IERR2,
     *IFRMAT
      SAVE/PCCOM1/
      INTEGERIPRE1,IPRE2,RATIO
      COMMON/PCCOM3/IPRE1,IPRE2,RATIO
      SAVE/PCCOM3/
      INTEGERISRT,IJASGN,ISX0,IFORM
      COMMON/PCVERI/ISRT,IJASGN,ISX0,IFORM
      SAVE/PCVERI/
      INTEGERNADR,KRY2,INADR,IJACPY
      CALLPCUNPK(N,IPARM,RPARM)
      IF(IERR.NE.0)THEN
      CALLPCPACK(IPARM,RPARM)
      RETURN
      ENDIF
      CALLPCVERF(N,IJA,X)
      IF(IERR.EQ.20)THEN
      CALLPCDSLV(N,A,RHS,X)
      CALLPCPACK(IPARM,RPARM)
      RETURN
      ELSEIF(IERR.NE.0)THEN
      CALLPCPACK(IPARM,RPARM)
      RETURN
      ENDIF
      KRY2=7
      IF(NSP*RATIO.LT.KRY2)THEN
      IERR=15
      MNEED=KRY2-NSP*RATIO
      CALLPCPACK(IPARM,RPARM)
      RETURN
      ENDIF
      INADR=KRY2+1
      NADR=(INADR+RATIO-2)/RATIO+1
      IF(IJASGN.EQ.0)THEN
      IJACPY=INADR
      INADR=IJACPY+N
      NADR=(INADR+RATIO-2)/RATIO+1
      IF(NADR.GT.NSP)THEN
      IERR=15
      MNEED=NADR-NSP
      CALLPCPACK(IPARM,RPARM)
      RETURN
      ENDIF
      CALLPCICPY(N,IJA,ISP(IJACPY))
      ENDIF
      ISP(1)=NSP+1-NADR
      ISP(2)=NADR
      CALLPCDRVX(N,IJA,A,RHS,X,ISP,RSP,METHOD,PRECON,PCSTOP,PCMV)
      MNEED=MNEED+NADR-1
      IF(IJASGN.EQ.0)THEN
      CALLPCICPY(N,ISP(IJACPY),IJA)
      ENDIF
      IPARM(64)=ISP(5)
      IPARM(65)=ISP(6)
      CALLPCPACK(IPARM,RPARM)
      RETURN
      END
