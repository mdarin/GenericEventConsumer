#!/bin/sh
# FILE: "D:\project\ikar2\examples\GenericEventConsumer\bin\run.sh"
# LAST MODIFICATION: ", 24 06 2010 14:48:58 Московское время (зима) ()"
# (C) 2009 by Bryukhovets Mikhail, <push.uni@gmail.com>
# vim:set sts=8 ts=8 sw=8:
# $Id:$

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../../lib ./GenericEventConsumer -ORBInitRef  ICCEventDispatcher15ktest=corbaloc::127.0.0.1:9994/ICCEventDispatcher
