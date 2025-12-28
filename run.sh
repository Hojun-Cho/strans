#!/bin/sh

pkill strans
pkill strans-xim

sleep 1
./strans map font &
sleep 1
xim/strans-xim &
