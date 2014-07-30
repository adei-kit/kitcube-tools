#include "hatpro.h"

hatpro::hatpro() {
    nSensorFlat = 0;
}

hatpro::~hatpro() {

}

    //=========================================================
    // variable counting
    // for 2d variables: 
    //      asume "time" sits in the first dimension
    //      second dimension will be flattened in database
    //
    // variable names
    // for 1d variables:
    //      use <sensor.name>
    // for flattened 2d variabls:
    //      use <sensor.name>.001, <sensor.name>.002, ...
    //=========================================================
    
void hatpro::openDatabase() {
#ifdef USE_MYSQL
	int i;
	MYSQL_RES *res;
	MYSQL_ROW row;
	std::string sql;
	const char *wild;
	char line[256];
	std::string cmd;
	int nNewSensors;
	int nNewAxis;
	
    
	
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
    
    // TODO: There is a modification for ORCAProcess when writing the sensorlist?!
    //      No idea how to configure this feature?!
	
	// TODO: error handling	
	// TODO:  Check if the data is available in the class?!
	
	// Connect to the database "<project>_active"
	// Create database, if not existing
	connectDatabase();
	
/*
	// allocate MYSQL object
	// TODO: only, if it does not exist yet!
	db = mysql_init(NULL);
	if (db == NULL) {
		printf("Error allocating MYSQL object!\n");
		// TODO: error handling
	}
	
	// Enable automatic reconnect
	my_bool re_conn = 1;
	mysql_options(db, MYSQL_OPT_RECONNECT, &re_conn);
	
	// connect to database
	printf("Database: \t\t%s@%s\n", dbUser.c_str(), dbHost.c_str());
	if (!mysql_real_connect(db, dbHost.c_str(), dbUser.c_str(), dbPassword.c_str(), NULL, 0, NULL, 0)) {
		printf("Failed to connect to database: %s\n", mysql_error(db));
		// TODO: error handling
	}
	
	// **********************************************************************
	//select the default database
	//create it, if it doesn't exist
	// **********************************************************************
	if (mysql_select_db(db, dbName.c_str())) {
		printf("Error selecting DB '%s' for use: %s\n", dbName.c_str(), mysql_error(db));
		
		printf("Creating DB '%s'...\n", dbName.c_str());
		sql = "CREATE DATABASE " + dbName;
		if (mysql_query(db, sql.c_str())) {
			printf("Error creating new DB '%s': %s\n", dbName.c_str(), mysql_error(db));
		}
		
		if (mysql_select_db(db, dbName.c_str())) {
			printf("Error selecting DB '%s' for use: %s\n", dbName.c_str(), mysql_error(db));
			// TODO: error handling
			return;
		}
	}
	
*/
	
	//----------------------------------------------------------------------
	// read list of databases, only kitcube* are relevant
	//----------------------------------------------------------------------
	// send SQL query
/*	sql = "SHOW databases";
	if (mysql_query(db, sql.c_str())) {
		printf("SQL command '%s' failed: %s\n", sql.c_str(), mysql_error(db));
		// TODO: error handling
	}
	
	// retrieve results
	res = mysql_store_result(db);
	if (res == NULL) {
		printf("Error retrieving results: %s\n", mysql_error(db));
		// TODO: error handling
	}*/
	
	wild = (project + "%").c_str();
	res = mysql_list_dbs(db, wild);
	if (res == NULL) {
		printf("Error retrieving database list: %s\n", mysql_error(db));
		// TODO: error handling
	}
	
	if (debug > 2) printf("Project \"%s\" database list: \t", project.c_str());
	
	// search for kitcube* database names
	while ((row = mysql_fetch_row(res)) != NULL) {
		if (strstr(row[0], project.c_str()) == row[0])
			if (debug > 2) printf("%s ", row[0]);
	}
	if (debug > 2) printf("\n");
	
	mysql_free_result(res);	

	
	// Create axis table
	// Read axis definition from inifile
	if (axis == 0)
		readAxis(this->inifile.c_str());

	// Check if axis table is available
	// Get list of cols in data table
	sql = "SHOW COLUMNS FROM " + axisTableName;
	if (mysql_query(db, sql.c_str())) {
		fprintf(stderr, "%s\n", sql.c_str());
		fprintf(stderr, "%s\n", mysql_error(db));
		
		// Create table
		printf("Creating axis table\n");
		cmd = "CREATE TABLE `";
		cmd += axisTableName;
		cmd += "` ( `id` int(10) auto_increment, ";
		cmd += "    `name` varchar(4), ";
		cmd += "    `comment` text, ";
		cmd += "    `unit` text, ";
		cmd += "    `min`  float(10), ";
		cmd += "    `max`  float(10), ";
		cmd += "PRIMARY KEY (`id`), INDEX(`name`) ) ENGINE=MyISAM";
		
		//printf("SQL: %s\n", cmd.c_str());
		
		if (mysql_query(db, cmd.c_str())) {
			fprintf(stderr, "%s\n", cmd.c_str());
			fprintf(stderr, "%s\n", mysql_error(db));
			
			throw std::invalid_argument("Creation of axis table failed");
		}
	}
	res = mysql_store_result(db);
	mysql_free_result(res);
	
	// Query all axis
	sql = "SELECT id, name FROM " + axisTableName;
	if (mysql_query(db, sql.c_str())) {
		fprintf(stderr, "%s\n", sql.c_str());
		fprintf(stderr, "%s\n", mysql_error(db));
		
		throw std::invalid_argument("Can not read axis table");
	}
	
	
	// Find missing axis definitions
	res = mysql_store_result(db);
	while ((row = mysql_fetch_row(res)) != NULL) {
		for (i = 0; i < nAxis; i++) {
			if (axis[i].name == row[1]){
				axis[i].isNew = false;
				axis[i].id = atoi(row[0]);
				if (debug > 2)
					printf("The axis %s is already defined in the axis list (id=%d)\n",
					       axis[i].name.c_str(), axis[i].id);
				break;
			}
		}
		//for (i=0;i<mysql_num_fields(res); i++){
		//	printf("%s", row[i]);
		//}
	}
	mysql_free_result(res);
	if (debug > 2) printf("\n");
	
	// Add the new axis definitions to the database
	nNewAxis = 0;
	for (i = 0; i < nAxis; i++) {
		if (axis[i].isNew) {
			if (debug) printf("Adding axis %s -- %s (%s) to the axis list\n",
				   axis[i].name.c_str(), axis[i].desc.c_str(), axis[i].unit.c_str());
			sql = "INSERT INTO " + axisTableName + "(`name`, `comment`, `unit`) VALUES ('" +
			      axis[i].name + "', '" + axis[i].desc + "', '" + axis[i].unit + "')";
			if (mysql_query(db, sql.c_str())) {
				fprintf(stderr, "%s\n", sql.c_str());
				fprintf(stderr, "%s\n", mysql_error(db));
				
				throw std::invalid_argument("Cannot create new axis entry");
			}
			
			// Get the id of the new sensor entry
			axis[i].id = mysql_insert_id(db);
			nNewAxis++;
		}
	}
	if ((debug) && (nNewAxis>0))
		printf("New axis definitions: %d\n", nNewAxis);
	
	
	/***********************************************************************
	 * create data table, if it doesn't exist
	 **********************************************************************/
	// Create Database tables
	create_data_table_name(dataTableName);
	create_data_table();
	
	
	// TODO: Check the sensor configuration
	
	
	// Create sensor list table
	// Get list of cols in data table
	sql ="SHOW COLUMNS FROM " + sensorTableName;
	if (mysql_query(db, sql.c_str())) {
		//fprintf(stderr, "%s\n", sql);
		//fprintf(stderr, "%s\n", mysql_error(db));
		
		// Create table
		printf("Creating sensor table\n");
		cmd = "CREATE TABLE `";
		cmd += sensorTableName;
		cmd += "` (`id` int(10) auto_increment, ";
		cmd += "`name` varchar(30) unique, ";
		cmd += "`module` int(10) unsigned default '0', ";
		cmd += "`comment` text, ";
		cmd += "`axis` int(10), ";
        cmd += "`axis2` int(10), ";
        cmd += "`axis2_idx` int(10), ";
        cmd += "`axis2_val` float(10) default '0', ";
		//cmd += "`height` float(10)  default '0', ";
		cmd += "`data_format` text, ";
		cmd += "PRIMARY KEY (`id`), INDEX(`name`) ) ENGINE=MyISAM"; 
		
		//printf("SQL: %s\n", cmd.c_str());
		
		if (mysql_query(db, cmd.c_str())){
			fprintf(stderr, "%s\n", cmd.c_str());
			fprintf(stderr, "%s\n", mysql_error(db));
			
			throw std::invalid_argument("Creation of table for sensor list failed");
		}
	}
	res = mysql_store_result(db);
	mysql_free_result(res);
	
	
	// TODO: Check if the sensor names are in the list
	// Store in sensor table
	// Bug: Is not stored in the first time?! At this time the database is not open...
	nNewSensors = 0;
	for (i = 0; i < nSensors; i++) {
		cmd = "INSERT INTO ";
		cmd += "`" + sensorTableName + "` ";
		cmd += "(`name`, `module`, `comment`, `axis`, `axis2`, `axis2_idx`, `axis2_val`, `data_format`) VALUES (";
		cmd += "'" + sensor[i].name + "', ";
		sprintf(line, "'%d', ", moduleNumber);
		cmd += line;
		cmd += "'" + sensor[i].name + " " + sensor[i].comment + "', ";
        //cmd += "'" + sensor[i].comment + " [" + sensor[i].name + "]', "; // modified for OrcaProcess, ak !!!
		sprintf(line, "'%d', '%d', ", axis[sensor[i].axis].id, axis[sensor[i].axis2].id);
		cmd += line;
        sprintf(line, "'%d', '%f', ", sensor[i].axis2_idx, sensor[i].axis2_val);
		cmd += line;
		cmd += "'" + sensor[i].data_format + "')";
		
		//printf("SQL: %s (db = %d)\n", cmd.c_str(), db);
		
		if (mysql_query(db, cmd.c_str())){
			//fprintf(stderr, "%s\n", cmd.c_str());
			//fprintf(stderr, "%s\n", mysql_error(db));
			
			// TODO: If this operation fails do not proceed in the file?!
			//break;
		} else {
			nNewSensors++;
		}
	}
	
	if ((debug) && (nNewSensors > 0)){ 
		printf("New sensors: \t\t%d\n", nNewSensors);
		printf("\n");
	}
	
    res = mysql_store_result(db);
	mysql_free_result(res);

	
    // Create status table
    openStatusTab();
    
	
	// Register in the status list
    registerStatusTab("Starting");
	
	
#endif // USE_MYSQL
}

int hatpro::create_data_table() {
#ifdef USE_MYSQL
	MYSQL_RES *result;
	MYSQL_ROW row;
	std::string sql_stmt;
	
	
	// search for tables with names like "dataTableName"
	result = mysql_list_tables(db, dataTableName.c_str());
	if (result == NULL) {
		printf("Error retrieving table list: %s\n", mysql_error(db));
		// TODO: error handling
	}
	
	// fetch row from result set
	row = mysql_fetch_row(result);
	
	// free memory for result set
	mysql_free_result(result);
	
	// if there is no row, meaning no table, create it
	if (row == NULL) {
		if (debug) printf("Creating data table %s...\n", dataTableName.c_str());
		
		// build SQL statement
		sql_stmt = "CREATE TABLE `" + dataTableName + "` ";
		if (useTicks)
			sql_stmt += "(`id` bigint auto_increment, `usec` bigint default '0', ";
		else
			sql_stmt += "(`id` bigint auto_increment, `sec` bigint default '0', ";
		for (int i = 0; i < nSensors; i++) { /* modified by CM for flattened 2-d data */
			if (sensor[i].type == "profile") {
				sql_stmt += "`" + sensor[i].name + "` blob, ";
			} else {
				sql_stmt += "`" + sensor[i].name + "` double, ";
			}
        }
		if (useTicks)
			sql_stmt += "PRIMARY KEY (`id`), INDEX(`usec`) ) ENGINE=MyISAM";
		else
			sql_stmt += "PRIMARY KEY (`id`), INDEX(`sec`) ) ENGINE=MyISAM";
		
		// execute SQL statement
		if (mysql_query(db, sql_stmt.c_str())) {
			printf("Error creating data table %s: %s\n", dataTableName.c_str(), mysql_error(db));
			// TODO: error handling
		}
	}
#endif
	return 0;
}

unsigned int hatpro::getSensorGroup() {
	unsigned int number;

    number = 0;
    buffer = "";

    if (sensorGroup == "L1B") {
        buffer = "raw data";
        number = 0;
    }

    if (sensorGroup == "L2A") {
        buffer = "raw data";
        number = 1;
    }

    if (sensorGroup == "L2C") {
        buffer = "raw data";
        number = 2;
    }

    return number;
}

int hatpro::readHeader(const char *filename) {
    int i, j, k, sum_sensors, *sensor_ct;
    float *values;
    struct sensorType *sensor_0;
    NcVar *var;
    char num[4];

	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);

    getSensorNames(sensorListfile.c_str());

    if (debug > 2) printf("%d sensor records read from sensor file\n", nSensors);

    NcFile dataFile(filename);

    sensor_0 = new struct sensorType [nSensors]; 
    sensor_ct = new int [nSensors];
    sum_sensors = 0;
    
    for (i = 0; i < nSensors; i++) {
        var = dataFile.get_var(sensor[i].comment.c_str());

        if (var->num_dims() == 1) {
            sensor_ct[i] = 1;
        } else {
            sensor_ct[i] = var->get_dim(1)->size();
        }

        sum_sensors += sensor_ct[i];

        sensor_0[i] = sensor[i];
    } 

    delete [] sensor;
    sensor = new struct sensorType [sum_sensors];

    k = 0;
    for (i = 0; i < nSensors; i++) {
        if (sensor_ct[i] == 1) {
            sensor[k] = sensor_0[i];
            sensor[k].data_format = "<1d>";
            sensor[k].axis2_idx = 0;
            sensor[k].axis2_val = 0;
            k++;
        } else {
            var = dataFile.get_var(sensor_0[i].axis2_name.c_str());

            values = new float [sensor_ct[i]];
            var->get(values, sensor_ct[i]);

            for (j = 0; j < sensor_ct[i]; j++) {
                sensor[k] = sensor_0[i];
                sensor[k].data_format = "<2d>";
                sensor[k].axis2_idx = j;
                sensor[k].axis2_val = values[j];

                //printf("sensor axis2 value %f\n", sensor[k].axis2_val);

                if (j < 10) 
                    sprintf(&num[0], ".00%d", j);
                else if (j <100)
                    sprintf(&num[0], ".0%d", j);
                else
                    sprintf(&num[0], ".%d", j);
                sensor[k].name = sensor_0[i].name + num;

                k++;
            }
        }
    }

    nSensors = sum_sensors;

    if (debug > 3) printf("%d variables will be loaded!\n", nSensors);

    delete [] sensor_0;
    delete [] sensor_ct;

	return 0;
}

void hatpro::readData(std::string filename) {
    int i, j;
    long time_sz;
    NcVar *ncvar, **var_list;
    struct timeval* time_stamps;
    double *time_data, **sensor_data;
    std::string sql;
    int sql_status;
	struct timeval lastTime;
	unsigned long lastPos;
	long lastIndex;
    
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);

    // open netcdf file
    if (debug >=1) 
        printf("opening netCDF file ...\n");
    NcFile dataFile(filename.c_str());
    if (! dataFile.is_valid()) {
        printf("\nFailed to open NetCDF file: %s\n", filename.c_str());
    }

    if (debug > 2)
        printf("nSensors = %d\n", nSensors);

    // If number of sensors is unknown read the header first
    if (nSensors == 0) 
        readHeader(filename.c_str());
	
#ifdef USE_MYSQL
    if ((db == 0) || (mysql_ping(db) != 0))
        openDatabase();
    if (db <= 0) {
         printf("Error: No database availabe\n");
        throw std::invalid_argument("No database");
    }
#endif


    // parse time dimension ---> timeval* 
    ncvar = dataFile.get_var("time_since_19700101");

    time_sz = ncvar->edges()[0];
    time_data = new double[time_sz];
    time_stamps = new timeval[time_sz];

    ncvar->get(time_data, time_sz);

	for (i = 0; i < time_sz; i++) {
        time_stamps[i].tv_sec = (int)time_data[i];
        time_stamps[i].tv_usec = (int)((time_data[i] - (double)time_stamps[i].tv_sec) * 1000000. + 0.5);	
    }

    // check last read position
    loadFilePosition(lastIndex, lastPos, lastTime);
    /*
    if ((int)lastPos = time_sz - 1) {
        printf("no new data from current file.\n");
    } else if ((int)lastPos > time_sz) {
        printf("last array position in record contradict with current array size\n");
    } else 
    */
   

    // read data from netCDF file
    sensor_data = new double* [nSensors];

    for (i = 0; i < nSensors; i++) {
        sensor_data[i] = new double [time_sz];
        memset(sensor_data[i], 0 ,time_sz*sizeof(double));
    }

    for (i = 0; i < nSensors; i++) {
        ncvar = dataFile.get_var(sensor[i].comment.c_str());

        if (ncvar->num_dims() == 1) {
            ncvar->get(sensor_data[i], time_sz);
        } else {
            j = sensor[i].axis2_idx;
            /*
            printf("\ti = %d    j = %d", i, j);
            printf("\tdim = %d", ncvar->num_dims());
            printf("\tn_vals = %d", ncvar->num_vals());
            printf("\ttime_sz = %d", time_sz);
            printf("\tdim0 = %d, dim1 = %d", ncvar->get_dim(0)->size(),ncvar->get_dim(1)->size());
            */
            int chk1 = ncvar->set_cur(0,j);
            int chk2 = ncvar->get(sensor_data[i], time_sz, 1);
            /*
            printf("\tchk1 = %d    chk2 = %d\n", chk1 ,chk2);
            */
        }
    }


#ifdef USE_MYSQL 
    // Write dataset to database
    printf("Write record to database %s\n", dataTableName.c_str());
    char sData[100];
    struct timeval t1, t0;
    struct timezone tz;
    
    int tk = int(lastPos); // time ticker
    int endPos;

    sql_status = 0;

    if (debug > 2) {
        endPos = (time_sz < tk + 20) ? time_sz : (tk + 20);
    } else {
        endPos = time_sz;
    }

    for (; tk < endPos; tk++) {
        sql = "INSERT INTO `";
        sql += dataTableName + "` (`usec`";
        for (i = 0; i < nSensors; i++) {
            sql += ",`";
            sql += sensor[i].name;
            sql += "`";
        }
        sql += ") VALUES (";
        sprintf(sData, "%ld", time_stamps[tk].tv_sec * 1000000 + time_stamps[tk].tv_usec);
        sql += sData;
        for (i = 0; i < nSensors; i++) {
            sprintf(sData, "%f", sensor_data[i][tk]);
            sql += ",";
            sql += sData;
        }
        sql += ")";
        
        if (debug > 5)
            printf("SQL: %s (db = %d)\n", sql.c_str(), db);
        
        gettimeofday(&t0, &tz);
        
        if (mysql_query(db, sql.c_str())) {
            sql_status = -1;
            break;
        }
        
        gettimeofday(&t1, &tz);
        printf("DB insert duration %ldus\n", (t1.tv_sec - t0.tv_sec)*1000000 + (t1.tv_usec-t0.tv_usec));

        saveFilePosition(lastIndex, tk+1, time_stamps[tk]);
    }

    if (tk >= time_sz) {
        fd_eof = true;
    } else {
        fd_eof = false;
    }

#endif

    for (i = 0; i < nSensors; i++)
        delete [] sensor_data[i];
    delete [] sensor_data;
    delete [] time_data;

#ifdef USE_MYSQL
    if (debug > 3)
        printf("sql_status = %d\n", sql_status);
    if (sql_status == -1) {
        fprintf(stderr, "%s\n", sql.c_str());
        fprintf(stderr, "%s\n", mysql_error(db));
        
        // If this operation fails do not proceed in the file?!
        printf("Error: Unable to write data to database\n");
        throw std::invalid_argument("Writing data failed");
    }

#endif
}

