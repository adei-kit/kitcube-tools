# KITcube configuration
# Central configuration file for shell scripts
# A. Kopmann (ak)
#

# Start script
#SERVICE_CAMPAIGN="HDCP2" 

# Backup
BACKUPHOST="katrin.kit.edu"
#BACKUPDIR=backup/$HOSTNAME
BACKUPEXCL=/home/cube/etc/rsync/backup-exclude

# Alarm system
ALARMRELAY=cube@katrin.kit.edu
#ALARMLOG=$KITCUBEDIR/alarm-$HOSTNAME-chk_alarm.log;
#ALARMLIST="tro.kitcube-alarm@imk.fzk.de";
#ALARMLIST="alarm@kitcube.kit.edu";
ALARMLIST="andreas.kopmann@kit.edu";
#ALARMSUBJ="KITcube Monitor -- Alarm status has changed";



STATRELAY=cube@katrin.kit.edu
#STATLIST="tro.kitcube-alarm@imk.fzk.de";
STATLIST="alarm@kitcube.kit.edu";
#STATLIST="andreas.kopmann@kit.edu";
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


# Sync_files.sh
SYNCUSEALARM="no"

# File formats (used by split_flatdir, ...)
FORMAT_CC="kitcube_cc2_%Y-%m-%d-%H-%M-%S.jpg"
FORMAT_CCSKY="%Y-%m-%d_%H-%M-%S_skypic.jpg"
FORMAT_CCLAND="%Y-%m-%d_%H-%M-%S_landpic.jpg"
FORMAT_CM="chm%Y%m%d_%H%M%S.dat"


