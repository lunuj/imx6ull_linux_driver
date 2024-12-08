#!/bin/bash
rmmod led
depmod
modprobe led
mknod /dev/led c 200 0
