**IMPORTANT: this information is deprecated as of release of FAT dingux. Please stay tuned for updates**

How to get linux up and running in your A320 in a few steps (from a Linux PC):

# Requirements #

  * usbtool: will let you control the console when in USB boot mode. Get a binary from the downloads section or get the source from [here](http://jz-hacking.googlecode.com/files/jz_tools_20090224.tar.bz2) and compile it yourself.
  * hwinit.bin: this is a minimal hardware initialization program.
  * zImage: this is the linux kernel.
  * rootfs.bz2: this is a minimal root file system.

# Prepare the root filesystem #

Get a miniSD card and make two partitions using fdisk or the program of your choice. Make sure the first parititon is type 0x0b (FAT32).

The size is not critical. The first partition is intended to store the linux kernel image, so as little as 2MB will do (but make it 64MB, flash is cheap). The second partition is intended to store the root filesystem. At the moment it is minimal, so you'll be fine with, say, 128MB, but make it as large as you wish.

Format both partitions (use your device instead of /dev/sdd1 and /dev/sdd2):

```
sudo mkfs.vfat -F 32 -n A320_VFAT /dev/sdd1
sudo mkfs.ext3 -L A320_EXT3 /dev/sdd2
```

Unpack the archive contents in the second partition:

```
sudo mount /dev/sdd2 /mnt/flash
sudo tar -C /mnt/flash -jxvf rootfs.tar.bz2
sudo umount /dev/sdd2
```

Yes... we have created and formatted a FAT32 parition that we are not going to use. This partition is from where u-boot loads the kernel image... when using u-boot. We are taking a shortcut and will be loading the kernel straight through USB, so we need not have the kernel image ready for u-boot in the first partition.

# Let's go #

Download [usbtool from rockbox](http://www.rockbox.org/twiki/bin/viewfile/Main/OndaVX747?rev=2;filename=usbtool).

Set the Dingoo A320 in USB boot mode by holding down button B while resetting. This tells the CPU to boot from ROM code which will initialize the USB interface and wait for commands from the PC. Note that at this stage the CPU initialization is minimal, and thus SDRAM is not even available. All we have is the instruction caché to upload and execute code (and it is very small, so we cannot load the linux kernel there).

When the A320 enters USB boot mode you will see no output on the screen, but if you run _dmesg_ in your PC console you should see something like this at the last lines:

```
[ 2018.565047] usb 1-3: new high speed USB device using ehci_hcd and address 11
[ 2018.698513] usb 1-3: configuration #1 chosen from 1 choice
```

Now upload the hardware initialization program which will set up SDRAM so we can place more stuff there:

```
sudo usbtool 1 hwinit.bin 0x80000000

[INFO] File size: 3144 bytes
[INFO] Searching for device...
[INFO] Found device, uploading application.
[INFO] GET_CPU_INFO: JZ4740V1
[INFO] SET_DATA_ADDRESS to 0x80000000... Done!
[INFO] Sending data... Done!
[INFO] Verifying data... Done!
[INFO] Booting device STAGE1... Done!
[INFO] Done!
```

Now upload the linux kernel itself:

```
sudo usbtool 1 zImage 0x80600000

[INFO] File size: 1220608 bytes
[INFO] Searching for device...
[INFO] Found device, uploading application.
[INFO] GET_CPU_INFO: JZ4740V1
[INFO] SET_DATA_ADDRESS to 0x80600000... Done!
[INFO] Sending data... Done!
[INFO] Verifying data... Done!
[INFO] Booting device STAGE1... Done!
[INFO] Done!
```

And that's it. You'll see the linux boot process on screen. At some point, your PC will recognize a new USB device (ACM communication device) and create /dev/ttyACM0. You can use minicom (57600 8N1, no flow control) to login into your console (user 'root', no password).

For those who are not familiar with minicom, some instructions:

Install minicom:

```
sudo apt-get install minicom
```

Create at $HOME a file named _.minirc.a320_ with this content:

```
pu port             /dev/ttyACM0
pu baudrate         57600
pu bits             8
pu parity           N
pu stopbits         1
pu rtscts           No
```

Now you can run the following command to communicate with your A320:

```
minicom a320
```


IMPORTANT: the console kernel will oops (the screen flashes) if you disconnect the USB cable. This is a known problem that is being investigated.

# How do I upload files? #

Sure, you have a serial console, but how do you upload your programs?. There are several ways to do it at the moment:

  * Do a normal reset and boot into the original firmware. You linux PC should see and mount the two partitions in the miniSD. Do file operations as you would do with any other storage device. When you're done, you'll have to to the linux kernel boot process again. This is the most convenient way, since you don't have to remove the miniSD.
  * Remove the miniSD, plug it into your PC (card reader needed, obviously), and alter its contents at will.
  * Use zmodem vía the serial console. You'll need to compile the rz program and send it to the Dingoo vía any of the two other file transfer methods.

# How do I compile programs? #

Get the toolchain from Ingenic FTP site or google for it (mipseltools-gcc412-glibc261.tar.bz2). Unpack it in /opt and add the binary directory to your path:

```
export PATH=/bin
```

For most automake enabled programs, you do something like this:

```
./configure --host=mipsel-linux --prefix=/
make
```

The minimal root filesystem contains libid3tag, libmad, libSDL, libSDL\_image and libSDL\_gfx, but for development you also need to install them (and include files) in the mips toolchain in your PC. Compile the libraries yourself or (better), just get the libs archive from the downloads section and unpack it in /opt/mipseltools-gcc412-glibc261 (properly configuring the SDL for compilation is a pain in the ass).

IMPORTANT: you need to set the environment variable SDL\_NOMOUSE or the SDL initialization in your programs will fail:

```
export SDL_NOMOUSE=1
```