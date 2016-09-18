#!/bin/sh
xset -dpms     # disable DPMS (Energy Star) features.
xset s off       # disable screen saver
xset s noblank # don't blank the video device
unclutter & #hide mouse pointer
matchbox-window-manager -use_cursor no -use_titlebar no  &
while true;do
# matchbox-keyboard &
midori -i 120 -e Fullscreen  -e ZoomOut -a http://localhost:5000
done
