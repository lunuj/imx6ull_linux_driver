#!/bin/bash
rmmod beep
depmod
modprobe beep
# mknod /dev/beep c 200 0
