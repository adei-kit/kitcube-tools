# KITcube configuration
# Central configuration file for shell scripts
# A. Kopmann (ak)
#


# Backup
BACKUPHOST="katrin.kit.edu"
#BACKUPDIR=backup/$HOSTNAME
BACKUPEXCL=/home/cube/etc/rsync/backup-exclude

# Alarm system
ALARMRELAY=cube@katrin.kit.edu
#ALARMLOG=$KITCUBEDIR/alarm-$HOSTNAME-chk_alarm.log;
#ALARMLIST="tro.kitcube-alarm@imk.fzk.de";
ALARMLIST="andreas.kopmann@kit.edu";
#ALARMSUBJ="KITcube Monitor -- Alarm status has changed";

STATRELAY=cube@katrin.kit.edu
#STATLIST="tro.kitcube-alarm@imk.fzk.de";
STATLIST="andreas.kopmann@kit.edu";
#STATSUBJ="KITcube Monitor -- Device Status Display";


# Status computer infrastructure
#CIARCHIVEDIR=/home/cube/archive
#CIMODNO=120
#CIMODID=SysCI
CITEMPHOST="gude-1"
CITEMPSENSOR="GUDEADS-EPC2X6-MIB::epc2x6Temperature.0"
#CITESTMODE="yes"
#CITEMPHOST="kopmann@ipepdvcompute1"
#CITEMPSENSOR="temp1"

