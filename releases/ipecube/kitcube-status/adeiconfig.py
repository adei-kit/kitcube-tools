# ADEI configuration for KITcube status
#


# ADEI 
#adei_url = 'http://imk-adei1/adei'
adei_url = 'http://ipepdv/adei-cube'
#adei_url = 'http://localhost:8024/adei'

# ADEI access parameters
adei_server = 'ipecube'
adei_database = 'kitcube_active'
#adei_server = 'kitcube'
#adei_database = 'hatzenbuehl'


# Generation of ADEI plots
use_virtualserver = '0' # Use virtual server for loggroup independant plots



# KITcube database
kcdb_server = 'ipepdvsrv1'
kcdb_name = 'kitcube_active'
#kcdb_server = 'imk-db1'
#kcdb_name = 'hatzenbuehl'
kcdb_user = 'cube'
kcdb_passwd = 'cube'

# Use JSON file that has been pushed to server?!
#kcdb_server =''
kcdb_file = 'data/modstat.json.sample'



# 045 Cloud camera
cc2_lastimage = 'data/CC2-latest.jpg'


# 120 Computer infrastructure
ci_hostlist = ("gude-1","gude-2")
#ci_file = "data/gude.json.sample" # only for testing



