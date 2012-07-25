# KITcube configuration
# Central configuration file for shell scripts
# A. Kopmann (ak)
#


# Backup
BACKUPHOST="katrin.kit.edu"
#BACKUPDIR=backup/$HOSTNAME
BACKUPEXCL=/home/cube/etc/rsync/backup-exclude

# Export
EXPORTARCHIVE=/home/cube/archive
EXPORTAGE=1

# Alarm system
ALARMRELAY=cube@katrin.kit.edu
#ALARMLOG=$KITCUBEDIR/alarm-$HOSTNAME-chk_alarm.log;
#ALARMLIST="tro.kitcube-alarm@imk.fzk.de";
ALARMLIST="andreas.kopmann@kit.edu";
ALARMSUBJ="KITcube Monitor (ipecube) -- Alarm status has changed";

STATRELAY=cube@katrin.kit.edu
#STATLIST="tro.kitcube-alarm@imk.fzk.de";
STATLIST="andreas.kopmann@kit.edu";
STATSUBJ="KITcube Monitor (ipecube) -- Device Status Display";


# Status computer infrastructure
#CIARCHIVEDIR=/home/cube/archive
#CIMODNO=120
#CIMODID=SysCI
CITESTMODE="yes"
#CITEMPHOST="gude-1"
#CITEMPSENSOR="GUDEADS-EPC2X6-MIB::epc2x6Temperature.0"
CITEMPHOST="kopmann@ipepdvcompute1"
CITEMPSENSOR="temp1"


