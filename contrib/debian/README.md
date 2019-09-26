
Debian
====================
This directory contains files used to package binariumd/binarium-qt
for Debian-based Linux systems. If you compile binariumd/binarium-qt yourself, there are some useful files here.

## binarium: URI support ##


binarium-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install binarium-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your binarium-qt binary to `/usr/bin`
and the `../../share/pixmaps/dash128.png` to `/usr/share/pixmaps`

binarium-qt.protocol (KDE)

