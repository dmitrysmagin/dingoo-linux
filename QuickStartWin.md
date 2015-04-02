**IMPORTANT: this information is deprecated as of release of FAT dingux. Please stay tuned for updates**

How to get linux up and running in your A320 in a few steps (from a Windows PC):

# Requirements #

  * USB\_Boot.exe: get it from [here](ftp://ftp.ingenic.cn/3sw/00tools/usb_boot/tools/usbboot1.4b-tools.zip) (Ingenic FTP site).
  * hwinit.bin: this is a minimal hardware initialization program.
  * zImage: this is the linux kernel.
  * rootfs.bz2: this is a minimal root file system.

# Previous reading #

Go read the QuickStart guide from linux and make sure you understand what's going on there.

# Prepare the root filesystem #

(sorry, I don't know a proper way to do this under Windows... maybe I'll find a way to make linux in the A320 mount a FAT32 partition as the rootfs)

# Let's go #

Download usb\_boot from the Ingenic FTP site and unpack it. Replace the included USBBoot.cfg file by the one you can find in the download section.

Copy zImage in the **same folder** where you unpacked the usb\_boot tool.

Now place the A320 in USB boot mode (reset while holding the B button pressed), launch the USB\_Boot.exe tool and execute these commands:

```
boot 0
load 0x80600000 zImage 0
go 0x80600000 0
```

You should see linux boot messages on the A320 LCD.

When the boot completes, linux will recognize a new hardware device, which is the CDC ACM serial port. You need a special driver to make this serial port work: get it [here](http://archive.gp2x.de/cgi-bin/cfiles.cgi?0,0,0,0,8,1191). Once installed, use hyperterminal (aaaaarg!!!) or any other serial port terminal program on the new serial port (57600 8N1, no RTS/CTS or XON/XOFF flow control).

# How do I upload files? #

(See the linux QuickStart)

# How do I compile programs? #

(To be completed)