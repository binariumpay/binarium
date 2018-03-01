#!/bin/sh
set -e
srcdir="$(dirname $0)"
#srcdir = "/media/rodion/SSD_Linux/Dash/dash/src/"
cd "$srcdir"
if [ -z ${LIBTOOLIZE} ] && GLIBTOOLIZE="`which glibtoolize 2>/dev/null`"; then
  LIBTOOLIZE="${GLIBTOOLIZE}"
  export LIBTOOLIZE
fi
autoreconf --install --force --warnings=all
