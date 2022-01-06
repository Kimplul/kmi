#!/bin/sh

if [ -e fs ]; then
	rm -r fs
fi

if [ -e rootfs.img ]; then
	rm rootfs.img
fi
