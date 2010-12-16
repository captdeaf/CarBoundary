#!/bin/bash

die() {
  echo $@ >&2
  exit 1
}

cd `dirname "$0"`
mypwd="`pwd`"

cd "$mypwd/libusb"
echo "Configuring libusb . . ."
sh autogen.sh >&/dev/null || die "unable to configure libusb: sh autogen.sh"
echo "Building libusb . . ."
make >&/dev/null || die "unable to compile libusb: make"
[ -f libusb-1.0 ] || ln -s libusb libusb-1.0

cd "$mypwd/libfreenect"
echo "Configuring libfreenect . . ."
cat > CMakeCache.txt <<EOT
LIBUSB_1_INCLUDE_DIR:FILEPATH=../libusb
LIBUSB_1_LIBRARY:STRING=usb-1.0 -L$mypwd/libusb/libusb/.libs
EOT
echo "Configuring libfreenect . . ."
cmake . >&/dev/null || die "Unable to configure libfreenect: cmake ."
echo "Building libfreenect . . ."
make >&/dev/null || die "Unable to build libfreenect: make"

echo "Everything is built!"
