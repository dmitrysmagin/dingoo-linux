#!/bin/sh

modprobe net2272

ARGV=( $ARGV )

if [ ${#ARGV[@]} -ge 2 ] && [ ${ARGV[1]} = "nand" ]; then
	mount -o remount,loop,ro /mnt/nand
	modprobe g_file_storage file=/dev/loop/7;
elif [ ${#ARGV[@]} -ge 2 ] && [ ${ARGV[1]} = "root" ]; then
	modprobe g_file_storage file=/dev/mtdblock/3;
else
	mount -o remount,loop,ro /mnt/sd
	modprobe g_file_storage file=/dev/mmcsd/disc0/disc;
fi

