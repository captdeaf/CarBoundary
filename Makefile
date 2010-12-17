LIBDIR="/opt/local/lib"

FREENECT_I=-Iext/libfreenect/include -Iext/libfreenect/wrappers/c_sync
LIBUSB_I=-Iext/libusb/libusb -Iext/libusb
SDL_I = $(shell sdl-config --cflags)

INC=$(FREENECT_I) $(LIBUSB_I) $(SDL_I) -I.

FREENECT =-Lext/libfreenect/lib -lfreenect -lfreenect_sync
LIBUSB =-Lext/libusb/libusb/.libs -lusb-1.0
SDL = $(shell sdl-config --libs) -lSDL_gfx
LIB=$(FREENECT) $(LIBUSB) $(SDL)

run: boundary_run
	sh run.sh

ext/libfreenect/lib/libfreenect.dylib:
	@echo "Building extensions . . ."
	sh ext/build-ext.sh

%.o: %.c
	$(CC) -c $(INC) $< -o $@

OBJS = boundary_main.o boundary_sdl.o

boundary_run: $(OBJS)
	$(CC) -o $@ $(LIB) $^

# DO NOT DELETE

boundary_main.o: boundary_sdl.h
boundary_sdl.o: boundary_sdl.h
