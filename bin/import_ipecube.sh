#!/bin/sh
#
# Import data from remote fileservers
#

# EB2 - all values from transport disk
/home/cube/bin/sync_offline.sh /home/cube/data/transport-disk/012/EB2/ /home/cube/archive/012/EB2 /home/cube/etc/rsync/mast-append-filter

