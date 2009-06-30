#!/bin/sh

rmmod g_file_storage
rmmod net2272

mount /proc
mount -o remount,sync,noatime /

ARGV=( $ARGV )

if [ ${#ARGV[@]} -ge 2 ] && [ ${ARGV[1]} = "nand" ]; then
	mount -t vfat -o remount,iocharset=utf8,noatime,sync,rw /dev/loop/7 /mnt/nand
elif [ ${#ARGV[@]} -ge 2 ] && [ ${ARGV[1]} = "root" ]; then
	echo "Not implemented"
else
	mount -t vfat -o remount,iocharset=utf8,noatime,sync,rw /dev/discs/disc0/part1 /mnt/sd
fi
