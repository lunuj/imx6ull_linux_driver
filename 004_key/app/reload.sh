#!/bin/bash
rmmod /dev/key
modprobe key
./key_test.out /dev/key
