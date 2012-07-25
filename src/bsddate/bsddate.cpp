// date.cpp
// 
//
//

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

main(int argc, char ** argv){

	const char *inFormat;
	const char *outFormat;
	const char *timeString;
	struct tm timestamp;
	char out[50];
	size_t res;

	if (argc < 4) {
		printf("BSD-like date\n");
		printf(" %s <input format> <string> <output format>\n", argv[0]);

		exit(0);
	}

	inFormat  = argv[1];
	timeString  = argv[2];
	outFormat = argv[3];

	// Parse string
        strptime(timeString, inFormat, &timestamp);	

	//printf("Time: %d.%d.%d\n", timestamp.tm_mday, timestamp.tm_mon+1, timestamp.tm_year+1900);

	res = strftime(out, 50, outFormat,  &timestamp);
	//printf("%s (res=%ld)\n", out, res);
	printf("%s\n", out);

}
