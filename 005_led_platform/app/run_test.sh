#!/bin/bash
rmmod leddriver
depmod
modprobe /dev/platform_led
./leddriver_test /dev/platform_led