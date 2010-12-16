#!/bin/bash

if [ ! -f boundary_run ] ; then
  set -x
  sh build.sh
  set +x
fi

export DYLD_LIBRARY_PATH=ext/libfreenect/lib:ext/libusb/libusb/.libs

$@ ./boundary_run
