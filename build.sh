#!/bin/bash

if [ ! -f ext/libfreenect/lib/libfreenect.dylib ] ; then
  echo "Building extensions . . ."
  sh ext/build-ext.sh
fi

LIBDIR="/opt/local/lib"

if [ ! -f "$LIBDIR/libSDL.dylib" ] ; then
  echo "Installing ports ..."
  sh install-ports.sh
fi

FREENECT_I="-Iext/libfreenect/include -Iext/libfreenect/wrappers/c_sync"
LIBUSB_I="-Iext/libusb/libusb -Iext/libusb"
SDL_I="`sdl-config --cflags`"

INC="$FREENECT_I $LIBUSB_I $SDL_I"

FREENECT="-Lext/libfreenect/lib -lfreenect -lfreenect_sync"
LIBUSB="-Lext/libusb/libusb/.libs -lusb-1.0"
SDL="`sdl-config --libs`"
LIB="$FREENECT $LIBUSB $SDL"

set -x
set -e
gcc -c boundary_main.c -o boundary_main.o $INC

gcc boundary_main.o -o boundary_run $LIB
