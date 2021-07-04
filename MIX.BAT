rem if exist anldiv.obj goto gotdiv
rem lib c:\c\lib\slibce *anldiv *anlmul *anlrem;
rem :gotdiv
rem nmk OPTS=real
pc /ia:\exm\headers;a:\exm\libs hexcalc.c
rem e2m hexcalc
