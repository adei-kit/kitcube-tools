#!/bin/sh
#
# Start synchronization using all three access modes
#

echo "__________Sync via LAN_________________"
/home/cube/bin/sync_append.sh 10.20.12.2:/usr/local/Rainbow5/rainbow/rawdata/143DEX /home/cube/archive/037/XR /home/cube/etc/rsync/xr-append-filter
echo
echo "__________Sync via WLAN________________"
/home/cube/bin/sync_append.sh 10.30.12.2:/usr/local/Rainbow5/rainbow/rawdata/143DEX /home/cube/archive/037/XR /home/cube/etc/rsync/xr-append-filter
echo
echo "__________Sync via UTM_________________"
/home/cube/bin/sync_append.sh 10.10.12.2:/usr/local/Rainbow5/rainbow/rawdata/143DEX /home/cube/archive/037/XR /home/cube/etc/rsync/xr-append-filter


