README.txt
A. Kopmann

The releases folder contains the campaign specific configuration for
the KITcube data aqcuisition. 


Transport scripts (rsync):
kitcube_conf.sh		Configuration of synchronization and backup
crontab			Backup of the crontab configuration
			Selection of modules
import_<loc>.sh		Script for manual import
			Alternative to crontab
<mod>-append-filters-<grp> filter masks


kitcube-tools:
kitcube.ini		Configuration of data file readers
*.sensors 		Sensor maps 	


kitcube-status:
adeiconfig.py		Configuration of data sources
getadei<mod>.cgi	Modules
kitcube-status.html	Web application 



Available configurations:
ipecube			Test setup in IPE
hatzenbuehl		First campagne with the KITcube prototype

corte			Corse 08/12
inra			Corse 08/12 secondary fileserver



