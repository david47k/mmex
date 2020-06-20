/*  mmex.c

    Resource extractor for the following files:
		.MMP,.PIC		MMFW Pictures
		.MMB			MMFW Blobs
		.MMA,.SND		MMFW Sounds
		.MMF			MMFW Films
		.MMS			MMFW 3 Script (of type MM)
		.VEC			Vector File Version 1.0
   
    Related file types, but not extracted by this program:
		.MMS			MMFW 3 Script (of type II)
		.ITX			SKU Text Binary v2 
   
    Found in a number of software programs from the late 90s
    Found in software developed by ImageBuilder Software, Inc. https://www.mobygames.com/company/imagebuilder-software-inc
    Found in software developed by D****y Interactive
   
    This file format can vary a bit between different files, even though they have the same header.
    There is a -offset command line parameter which may help in some situations.

    The file format is big-endian.      

	Copyright 2020 David Atkinson
	Author: David Atkinson <dav!id47k@d47.co> (remove the '!')
	License: GNU General Public License version 2 or any later version (GPL-2.0-or-later)
	
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//----------------------------------------------------------------------------
//  BINARY FILE STRUCTURE
//----------------------------------------------------------------------------

uint8_t magicStr[] = { 'M','M','F','W',' ','P','i','c','t','u','r','e','s',  0,  0,  0,'M','M' };
// Can also be MMFW Blobs, MMFW Sounds, etc.

uint8_t magicStr2[] =   { 'V','e','c','t','o','r',' ','F','i','l','e',' ','V','e','r','s','i','o','n',' ','1','.','0' };
// A more unusual one

struct KNOWN_FILES {
	uint8_t magic[6];
	char name[16];
	uint32_t offset;
	uint8_t hasFilenames;
	char ext[5];
};

// The magic for known files starts at 0x14

struct KNOWN_FILES knownFiles[] = {
	{ {0x00,0x00,0x1e,0x49,0x35,0xCD}, "Lmps.pic", 			0x1A, 1, ".bin" },
	{ {0x45,0x02,0x9d,0x88,0x00,0x65}, "TarzanPI.mmp", 		0x22, 1, ".bin" },
	{ {0x3D,0x98,0x27,0x2B,0x00,0x65}, "ToyStory2PI.MMB",	0x22, 1, ".bin" },
	{ {0xB3,0x3B,0x6F,0xF6,0x00,0x00}, "Bugs.mmp", 			0x22, 1, ".bin" },
	{ {0x40,0x00,0x20,0xFC,0x9D,0x12}, "MUpsIntS.SND", 		0x1A, 1, ".bin" },
	{ {0x53,0xAC,0xA9,0x9A,0x00,0x01}, "Bugsai.mms", 		0x22, 0, ".bin" },
	{ {0x31,0x2E,0x30,0x00,0xFA,0x00}, "MUpsVec.VEC", 		0x17, 0, ".cgm" }
};

struct NAME {
    char str[32];
};

//----------------------------------------------------------------------------
//  DUMPRESOURCE: dump blob to disk
//----------------------------------------------------------------------------

#define BLOCKSIZE 4096

int dumpResource(FILE * fin, uint32_t offset, uint32_t byteCount, char * filename) {
    // Dumps a resource to disk

    // Open output file
    FILE * fout = fopen(filename,"wb");
    if(fout==NULL) {
		printf("dumpResource: Unable to open output file '%s'\n",filename);
		return 1;
    }

    // Save input file seek position
    long int oldPos = ftell(fin);

    // Seek to resource position
    if(fseek(fin,offset,SEEK_SET) != 0) {
		printf("dumpResource: fseek failed\n");
		return 1;
    }

    // Copy blocks
    char buf[BLOCKSIZE];
    for(uint32_t bytesLeft=byteCount; bytesLeft>0; ) {
		uint32_t count = BLOCKSIZE;
		if(bytesLeft<BLOCKSIZE) count = bytesLeft;

		if(fread(buf,1,count,fin) != count) {
			printf("dumpResource: fread failed\n");
			return 1;
		}

		if(fwrite(buf,1,count,fout) != count) {
			printf("dumpResource: fwrite failed\n");
			return 1;
		}
		bytesLeft -= count;
    }

    // Close output file
    fclose(fout);

    // Restore input file seek position
    if(fseek(fin,oldPos,SEEK_SET) != 0) {
		printf("dumpResource: fseek reset failed\n");
		return 1;
    }

    return 0;
}

//----------------------------------------------------------------------------
//  BYTE ORDER FUNCTIONS
//----------------------------------------------------------------------------

int systemIsLittleEndian(){
    volatile uint32_t i=0x01234567;
    // return 0 for big endian, 1 for little endian.
    return (*((uint8_t*)(&i))) == 0x67;
}

uint16_t reverseByteOrder2(uint16_t input) {
	if(!systemIsLittleEndian()) return input;
    uint16_t output = (input&0xFF)<<8;
    output |= (input&0xFF00)>>8;
    return output;
}

uint32_t reverseByteOrder4(uint32_t input) {
    if(!systemIsLittleEndian()) return input;
	uint32_t output = 0;
    output |= (input&0x000000FF) << 24;
    output |= (input&0x0000FF00) <<  8;
    output |= (input&0x00FF0000) >> 8;
    output |= (input&0xFF000000) >> 24;
    return output;
}

//----------------------------------------------------------------------------
//  MAIN
//----------------------------------------------------------------------------

int main(int argc, char * argv[]) {
    printf("%s\n","mmex: MMFW resource extractor");
    
	char * basename = "mmex";
	if(argc>0) {
		// find the name of the executable. not perfect but it's only used for display and no messy ifdefs.
		char * b = strrchr(argv[0],'\\');
		if(!b) {
			b = strrchr(argv[0],'/');
		}
		basename = b ? b+1 : argv[0];
	}

    if(argc<2) {
		printf("\nUsage: \n%s inputFile -offset hexOffset -dump prefix -usenames -ext extension\n\n",basename);
		printf("%s","inputFile               a compatible file. the only required parameter.\n");
		printf("%s","-offset hexOffset       specify the file offset where the 16-bit resource count\n"
					"                        is. e.g. -offset 1A\n");
		printf("%s","-dump prefix            dumps the files out with the specified prefix.\n"
					"                        e.g. -dump output_folder\\\n");
		printf("%s","-usenames               when dumping, use resource names as filenames.\n");
		printf("%s","-ext extension          when dumping, use the specified file extension.\n"
					"                        e.g. -ext .cgm\n");
		return 0;
    }
		
    char * filename = argv[1];
    char * outPrefix = "";
	char * outExt = ".bin";
    int dump = 0;
    int usenames = 0;
	uint32_t offset = 0x22;
	int offsetFound = 0;
    
	for(int i=2; i<argc; i++) {
		if(strncmp(argv[i],"-usenames",9)==0) {
			usenames = 1;
		} else if(strncmp(argv[i], "-dump",5)==0) {
			if(i != (argc-1)) {
				outPrefix = argv[i+1];
				dump = 1;
				i++;
				continue;
			} else {
				printf("Missing parameter for: -dump\n");
				return 1;
			}
		} else if(strncmp(argv[i], "-offset",7)==0) {
			if(i != (argc-1)) {
				sscanf(argv[i+1],"%x",&offset);
				printf("Using specified offset: 0x%X\n",offset);
				offsetFound = 1;
				i++;
				continue;
			} else {
				printf("Missing parameter for: -offset\n");
				return 1;
			}			
		} else {
			printf("Unknown parameter: %s\n",argv[i]);
		}
	}		
	
    if(strlen(outPrefix)>256) {
		printf("Output prefix is too long\n");
		return 1;
    }

    FILE * f = fopen(filename, "rb");

    if(f==NULL) {
		printf("Failed to open input file: %s\n", filename);
		return 1;
    }

    fseek(f,0,SEEK_SET);

    uint8_t buf[257]="";

    if(fread(buf,1,26,f) != 26) {
		printf("Read failed (header).\n");
		return 1;
    }

	int isVEC = 0;

    if(memcmp(buf,magicStr,5) != 0 || memcmp(&buf[0x0F],"\0MM",3) != 0) {		/* There are lots of 'MMFW '* types */
		if(memcmp(buf,magicStr2,sizeof(magicStr2)) != 0) {
			printf("Not a recognised MMFW file\n");
			return 1;
		} else {
			isVEC = 1;
			offset = 0x17;
			offsetFound = 1;
		}			
    }
	
	if(isVEC) {
		printf("File header: Vector File Version 1.0\n");
	} else {
		printf("File header: %s\n", buf);
	}

	uint16_t mmVersion = reverseByteOrder2(*(uint16_t*)(&buf[0x12]));
	printf("MMFW version: %u\n",mmVersion);

	uint8_t * fileID = &buf[0x14];
	
	int knownFilesSize = sizeof(knownFiles) / sizeof(struct KNOWN_FILES);

	for(int i=0; i<knownFilesSize; i++) {
		if(memcmp(fileID,knownFiles[i].magic,6)==0) {
			offset = knownFiles[i].offset;
			offsetFound = 1;
			outExt = knownFiles[i].ext;
			printf("Recognised file: %s, using offset: 0x%X\n", knownFiles[i].name, offset);
			break;
		}
	}

	if(!offsetFound) {
		printf("Using default offset: 0x%02X\n",offset);
	}

	fseek(f,offset,SEEK_SET);

    // Read in the resource count
    uint16_t resCount = 0;
    if(fread(&resCount,1,2,f) != 2) {
		printf("Read failed (count).\n");
		return 1;
    }
    resCount = reverseByteOrder2(resCount);
    resCount ++; 						// the last offset is the EOF offset
    printf("Resource count: %u\n",resCount-1);
    if(resCount == 0) {
		printf("No resources!\n");
		return 1;
    }
	
	// Read in the resource offsets
    uint32_t * offsets;
	uint32_t * sizes = NULL;
    offsets = malloc(sizeof(uint32_t) * resCount);
    if(offsets == NULL) {
		printf("Not enough memory (offsets)\n");
		return 1;
    }
	
	if(isVEC) {
	    sizes = malloc(sizeof(uint32_t) * resCount);
		if(sizes == NULL) {
			printf("Not enough memory (sizes)\n");
			return 1;
		}
	}

    for(uint16_t i=0; i<resCount; i++) {	
		uint32_t num;
		if(fread(&num,4,1,f) != 1) {
			printf("Read failed (offsets)\n");
			return 1;
		}
		offsets[i] = reverseByteOrder4(num);
		if(isVEC) {
			if(fread(&num,4,1,f) != 1) {
				printf("Read failed (sizes)\n");
				return 1;
			}
			sizes[i] = reverseByteOrder4(num);
		}
    }

	if(offset != 0x1A) {
		fseek(f,2,SEEK_CUR); 			// SKIP a null 2 bytes
	}

	// At this point we can figure out if there are names or not
	// Each name is 32 bytes, so we will need at least 32*resCount bytes before the first offset
	int gap = offsets[0] - ftell(f);
	int hasNames = 0;
	if(gap >= 32*(resCount-1)) {
		hasNames = 1;
	}
	printf("Has names: %i\n",hasNames);
	
    struct NAME * names = NULL;
    
	if(hasNames) {
		names = malloc(resCount * sizeof(struct NAME));
		if(names == NULL) {
			printf("Not enough memory (names)\n");
			return 1;
		}

		for(uint16_t i=0; i<resCount; i++) {	
			if(fread(&names[i],sizeof(struct NAME),1,f) != 1) {
				printf("Read failed (names)\n");
				return 1;
			}
		}
	}
	    
    for(uint16_t i=0; i<(resCount-1); i++) {
		uint32_t size = offsets[i+1] - offsets[i];
		if(isVEC) {
			size = sizes[i];			
		}

		printf("block %05u offset 0x%08X size 0x%08X label '%s' ", i, offsets[i], size, (hasNames?names[i].str:""));
		if(dump) {
			char filename[513];
			if(!usenames || !hasNames) {
				sprintf(filename,"%s%05u%s",outPrefix,i,outExt);
			} else {
				sprintf(filename,"%s%s",outPrefix,names[i].str);
			}
			if(dumpResource(f,offsets[i],size,filename) != 0) {
				printf("\ndumpResource failed\n");
				return 1;
			}
			printf("dumped to '%s'",filename);
		}
		printf("\n");
    }	

    fclose(f);

    return 0;
}