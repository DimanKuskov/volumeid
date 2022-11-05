#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define max_pbsi 3

struct partial_boot_sector_info
	{
		char* Fs; // file system name
		int FsOffs; // offset of file system name in the boot sector
		int SerialOffs; // offset of the serialnumber in the boot sector
	};


const struct partial_boot_sector_info pbsi[max_pbsi] =
	{
		{"FAT32", 0x52, 0x43},
		{"FAT",   0x36, 0x27},
		{"NTFS",  0x03, 0x48}
	};

void usage(char* cmdname) {
	printf("Usage: %s <device> [<new-id>]\n", cmdname);
}

int main(int argc, char** argv) {
	if(argc < 2 || argc > 4) {
		usage(argv[0]);
		return 1;
	}
	
	printf("%i\n", sizeof(int));
	
	unsigned int newId = 0;
	if(argc == 3) {
		char* end;
		newId = strtoul(argv[2], &end, 16);
		if(newId == 0 || end[0] != 0 || strlen(argv[2]) != 8) {
			fprintf(stderr, "New id must be 8 digit hex number\n");
			return 2;
		}
	}
	
	char* deviceName = argv[1];
	FILE* device = fopen(deviceName, "r+b");
	if(device == NULL) {
		perror("Unable to open device");
		return 3;		
	}
	
	int returnCode = 0;
	
	char sector[512];
	if(fread(sector, sizeof(char), sizeof(sector), device) != sizeof(sector)) {
		perror("Failed to read device");
		returnCode = 4;
		goto close;
	}

	int pbsiId;
	for (pbsiId=0; pbsiId<max_pbsi; pbsiId++) {
		if (strncmp(pbsi[pbsiId].Fs, &sector[pbsi[pbsiId].FsOffs], strlen(pbsi[pbsiId].Fs)) == 0) {
			break;
		}
	}

	if (pbsiId >= max_pbsi)
	{
		fprintf(stderr, "Failed to find FS signature\n");
		returnCode = 5;
		goto close;
	}
	
	printf("FileSystem is: %s\n", pbsi[pbsiId].Fs);
	unsigned int* volumeId = (int *)&sector[pbsi[pbsiId].SerialOffs];
	printf("Volume ID is: %X-%X\n", *volumeId >> 16, *volumeId & 0xFFFF);

	if(newId == 0) {
		goto close;
	}
	
	printf("New ID will be: %X-%X\n", newId >> 16, newId & 0xFFFF);
	
	*volumeId = newId;
	rewind(device);	
	if(fwrite(sector, sizeof(char), sizeof(sector), device) != sizeof(sector)) {
		perror("Failed to write to device");
		returnCode = 6;
		goto close;
	}	
	
	
	close: 
	if(device != NULL) {
		fclose(device);
	}
	return returnCode;
}
