#!/bin/bash

if [ ! -f ext/libfreenect/lib/libfreenect.dylib ] ; then
  echo "Building extensions . . ."
  sh ext/build-ext.sh
fi

if ! sdl-config --version ; then
  echo "Installing ports ..."
  sh install-ports.sh
fi

make
