#!/bin/sh
# List of sync_files.sh calls 
# Setup: hatzenbuehl 2012
#

SRCDIR=/home/cube/transfer-disk

#
# Liste of available devices
#
# 001/T01
#/home/cube/bin/sync_append.sh $SRCDIR/archive/001/T01/DATA/ /home/cube/archive/001/T01/DATA /home/cube/etc/rsync/mast-append-filter 

# 002/T02
#/home/cube/bin/sync_append.sh $SRCDIR/archive/002/T02/DATA/ /home/cube/archive/002/T02/DATA /home/cube/etc/rsync/mast-append-filter 

# 003/T03
#/home/cube/bin/sync_append.sh $SRCDIR/archive/003/T03/DATA/ /home/cube/archive/003/T03/DATA /home/cube/etc/rsync/mast-append-filter 

# 004/T04
#/home/cube/bin/sync_append.sh $SRCDIR/archive/004/T04/ /home/cube/archive/004/T04 /home/cube/etc/rsync/mast-append-filter 

# 005/T05
#/home/cube/bin/sync_append.sh $SRCDIR/archive/005/T05/DATA/ /home/cube/archive/005/T05/DATA /home/cube/etc/rsync/mast-append-filter 

# 006/T06
#/home/cube/bin/sync_append.sh $SRCDIR/archive/006/T06/DATA/ /home/cube/archive/006/T06/DATA /home/cube/etc/rsync/mast-append-filter 

# 007/T07
#/home/cube/bin/sync_append.sh $SRCDIR/archive/007/T07/DATA/ /home/cube/archive/007/T07/DATA /home/cube/etc/rsync/mast-append-filter 

# 011/EB1
#/home/cube/bin/sync_append.sh $SRCDIR/archive/011/EB1/DATA/ /home/cube/archive/011/EB1/DATA /home/cube/etc/rsync/mast-append-filter 
#/home/cube/bin/sync_append.sh $SRCDIR/archive/011/EB1/SISOMOP/ /home/cube/archive/011/EB1/SISOMOP /home/cube/etc/rsync/eb1-append-filter-sm 

# 012/EB2
/home/cube/bin/sync_append.sh $SRCDIR/archive/012/EB2/DATA/ /home/cube/archive/012/EB2/DATA /home/cube/etc/rsync/mast-append-filter 
/home/cube/bin/sync_append.sh $SRCDIR/archive/012/EB2/SISOMOP/ /home/cube/archive/012/EB2/SISOMOP /home/cube/etc/rsync/eb2-append-filter-sm 

# 013/PT20
#/home/cube/bin/sync_append.sh $SRCDIR/archive/001/T01/DATA/ /home/cube/archive/013/PT20/DATA /home/cube/etc/rsync/mast-append-filter 

# 020/CM Ceilometer
#/home/cube/bin/sync_files.sh $SRCDIR/archive/020/CM/ /home/cube/archive/020/CM /home/cube/etc/rsync/cm-append-filter CM 
#/home/cube/bin/sync_files.sh $SRCDIR/archive/020/CM/extra /home/cube/archive/020/CM/extra /home/cube/etc/rsync/cm-append-filter-chm 

# 025/JWD 
#/home/cube/bin/sync_files.sh $SRCDIR/archive/025/JWD/ /home/cube/archive/025/JWD /home/cube/etc/rsync/jwd-append-filter-chm 

# 030/PARS Parsivel
#/home/cube/bin/sync_append.sh $SRCDIR/archive/030/PARS/ /home/cube/archive/030/PARS /home/cube/etc/rsync/pars-append-filter 

# 035/MRR Radar
#/home/cube/bin/sync_append.sh $SRCDIR/archive/035/MRR/ /home/cube/archive/035/MRR /home/cube/etc/rsync/mrr-append-filter 

# 036/CR Cloud radar (MIRA36-S)
#/home/cube/bin/sync_files.sh $SRCDIR/archive/036/CR/ /home/cube/archive/036/CR /home/cube/etc/rsync/cr-append-filter  

# 037/XR X-band radar
/home/cube/bin/sync_files.sh $SRCDIR/archive/037/XR/ /home/cube/archive/037/XR /home/cube/etc/rsync/xr-append-filter 

# 040/RWP Regenwippe
#/home/cube/bin/sync_append.sh $SRCDIR/archive/040/RGW/ /home/cube/archive/040/RGW /home/cube/etc/rsync/wippe-append-filter 

# 045/CC2
#/home/cube/bin/sync_files.sh $SRCDIR/archive/045/CC2/data/ /home/cube/archive/045/CC2/data /home/cube/etc/rsync/cc-append-filter 

# 046/CC1
/home/cube/bin/sync_files.sh $SRCDIR/archive/046/CC1/data/ /home/cube/archive/046/CC1/data /home/cube/etc/rsync/cc-append-filter 

# 050/SCI
#/home/cube/bin/sync_append.sh $SRCDIR/archive/050/SCI/ /home/cube/archive/050/SCI /home/cube/etc/rsync/sci-append-filter 

# 060/HYB
/home/cube/bin/sync_append.sh $SRCDIR/archive/060/HYB/ /home/cube/archive/060/HYB /home/cube/etc/rsync/wtx-append-filter 

# 061/WTX
#/home/cube/bin/sync_files.sh $SRCDIR/archive/061/WTX/ /home/cube/archive/061/WTX /home/cube/etc/rsync/wtx-append-filter > /dev/null 2>&1

# 062/WC (before update)
#/home/cube/bin/sync_append.sh $SRCDIR/archive/062/WC/ /home/cube/archive/062/WC /home/cube/etc/rsync/wc-append-filter > /dev/null 2>&1

# 063/WC2 (after update in Jan 2012)
#/home/cube/bin/sync_append.sh $SRCDIR/archive/063/WC/ /home/cube/archive/063/WC /home/cube/etc/rsync/wc-append-filter > /dev/null 2>&1

# 065/GPS
#/home/cube/bin/sync_append.sh $SRCDIR/archive/065/GPS/ /home/cube/archive/065/GPS 

# 070/SOD
/home/cube/bin/sync_append.sh $SRCDIR/archive/070/SOD/ /home/cube/archive/070/SOD /home/cube/etc/rsync/sod-append-filter 
/home/cube/bin/sync_files.sh $SRCDIR/archive/070/SOD/ /home/cube/archive/070/SOD /home/cube/etc/rsync/sod-sync-filter 

# 080/HATPRO
#/home/cube/bin/sync_files.sh $SRCDIR/archive/080/RPG/data/ /home/cube/archive/080/RPG/data /home/cube/etc/rsync/rpg-append-filter 

# 081/HATPRO
/home/cube/bin/sync_files.sh $SRCDIR/archive/081/RPG/data/ /home/cube/archive/081/RPG/data /home/cube/etc/rsync/rpg-append-filter 

# 120/SysCI Status computer infrastructure
#/home/cube/bin/sync_files.sh $SRCDIR/archive/120/SysCI/ /home/cube/archive/120/SysCI /home/cube/etc/rsync/txt-append-filter





