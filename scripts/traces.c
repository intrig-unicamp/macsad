#include "controller.h"
#include "messages.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MAX_MACS 600000

uint8_t ips[MAX_MACS][4];
int mac_count = -1;

int read_files(char *filename) {
	printf("Reading FILE\n");
	FILE *f;
	char line[200];
	int values_ip[4];
	int i;

	f = fopen(filename, "r");
	if (f == NULL) return -1;

	while (fgets(line, sizeof(line), f)) {
		line[strlen(line)-1] = '\0';
		if (4 == sscanf(line, "%d.%d.%d.%d",
					&values_ip[0], &values_ip[1], &values_ip[2], &values_ip[3]) )
		{
			if (mac_count==MAX_MACS-1)
			{
				printf("Too many entries...\n");
				break;
			}

			++mac_count;
			for( i = 0; i < 4; ++i )
				ips[mac_count][i] = (uint8_t) values_ip[i];

		} else {
			printf("Wrong format error in line %d : %s\n", mac_count+2, line);
			fclose(f);
			return -1;
		}

	}

	fclose(f);
	return 0;
}

int main()
{
	read_files("nfpa.trL3_100_random.nfo");
	char data[100] = "";
	int i;
	for (i=0;i<=mac_count;++i)
	{
		//printf("Adding IP: %d.%d.%d.%d\n", ips[i][0],ips[i][1],ips[i][2],ips[i][3]);
		//sprintf(data, sizeof(data),ips[i][0],".",ips[i][1],".",ips[i][2],".",ips[i][3]," 0xa0:0x36:0x9f:0x3e:0x94:0xea 1");
		snprintf ( data, 100, "echo %d.%d.%d.%d  0xa0:0x36:0x9f:0x3e:0x94:0xea 1 >> /root/Fabricio/mac/trace_100.txt", ips[i][0],ips[i][1],ips[i][2],ips[i][3] );
		//printf("%s\n", data);
		system(data);
	}
}