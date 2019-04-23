# usb.dst == "1.5.0" || usb.src== "1.5.0"
# usb.dst == "1.7.0" || usb.src== "1.7.0"
CC = gcc
CFLAGS = -I/usr/include/libusb-1.0 -L/lib -DLINUX
LDFLAGS = -lusb-1.0

all:
	$(CC) -o ch9328_hid ch9328_hid.c $(CFLAGS) $(LDFLAGS)
