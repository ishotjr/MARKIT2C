rem requied for running from APP200:
rem cd c:\dev\markit2c

rem only on 200lx
c:\turbo\maxcpu

if exist anldiv.obj goto gotdiv

rem lib c:\c\lib\slibce *anldiv *anlmul *anlrem;

rem corrected path:
rem
lib a:\c600\lib\slibce *anldiv *anlmul *anlrem;


:gotdiv
nmk OPTS=real
e2m markit2c

rem only on 200lx
c:\turbo\normcpu
