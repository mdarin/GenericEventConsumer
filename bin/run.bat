::LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../lib ./eventdispatcher ../../icfg $1
@echo off
set PATH=%PATH%;..\..\..\lib\
GenericEventConsumer %1 %2 %3 %4
