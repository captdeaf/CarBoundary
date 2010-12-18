OBJS = boundary_main.o boundary_sdl.o
BINARY = boundary_run

CFLAGS = -ggdb -Wall

FREENECT_I=-Iext/libfreenect/include -Iext/libfreenect/wrappers/c_sync
LIBUSB_I=-Iext/libusb/libusb -Iext/libusb
SDL_I = $(shell sdl-config --cflags)

INC=$(FREENECT_I) $(LIBUSB_I) $(SDL_I) -I.

FREENECT =-Lext/libfreenect/lib -lfreenect -lfreenect_sync
LIBUSB =-Lext/libusb/libusb/.libs -lusb-1.0
SDL = $(shell sdl-config --libs) -lSDL_gfx
LIB=$(FREENECT) $(LIBUSB) $(SDL)

run: $(BINARY)
	sh run.sh

ext/libfreenect/lib/libfreenect.dylib:
	@echo "Building extensions . . ."
	sh ext/build-ext.sh

%.o: %.c
	$(CC) -c $(INC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJS) $(BINARY)

$(BINARY): $(OBJS)
	$(CC) -o $@ $(LIB) $^

# DO NOT DELETE

boundary_main.o: boundary_sdl.h
boundary_main.o: device_definitions.h
boundary_sdl.o: boundary_sdl.h
