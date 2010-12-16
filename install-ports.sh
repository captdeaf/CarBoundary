#!/bin/bash

if [ "`whoami`" != "root" ] ; then
  echo "$0: Need sudo to install this."
  exec sudo sh $0 $@
fi

port install cmake
port install libsdl
port install libsdl_gfx
