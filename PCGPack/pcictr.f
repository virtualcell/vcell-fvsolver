      SUBROUTINEPCICTR(N,IJL,IJU,LINK)
      IMPLICITDOUBLEPRECISION(A-H,O-Z)
      INTEGERN,IJL(*),IJU(*),LINK(*)
      INTEGERI,J
      DO1I=1,N
      DO3J=IJL(I),IJL(I+1)-1
      IJU(J)=I
3     CONTINUE
1     CONTINUE
      CALLPCICTX(N,IJL,IJU,LINK)
      RETURN
      END
