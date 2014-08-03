# ADEI configuration for KITcube status
#


# ADEI 
adei_url = 'http://imk-adei1/adei'
#adei_url = 'http://localhost:8024/adei'

# ADEI access parameters
adei_server = 'kitcube'
adei_database = 'HEADS'

# Generation of ADEI plots
use_virtualserver = '0' # Use virtual server for loggroup independant plots



# KITcube database
kcdb_server = 'imk-db1'
kcdb_name = 'HEADS'
kcdb_user = 'cube'
kcdb_passwd = 'cube'

# Use JSON file that has been pushed to server?!
#kcdb_server =''
kcdb_file = 'data/modstat.json.sample'


# 037 X-band radar
xr_lastimage = 'http://sop.hymex.org/archive/kolher/lastfile.png'

# 045 Cloud camera
cc2_lastimage = 'data/CC-latest.jpg'

# 075 Radiosonde
rs_g_rs = "Data_075_RS1_txt"

# 120 Computer infrastructure
ci_hostlist = ("gude-1","gude-2")
#ci_file = "data/gude.json.sample" # only for testing



