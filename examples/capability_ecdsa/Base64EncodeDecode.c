#include "Base64EncodeDecode.h"

static int decodeblock(char *input, unsigned char *output, int oplen)
{
 unsigned char decodedstr[ENCODERLEN + 1] = "";
 decodedstr[0] = input[0] << 2 | input[1] >> 4;
 decodedstr[1] = input[1] << 4 | input[2] >> 2;
 decodedstr[2] = input[2] << 6 | input[3] >> 0;
 uint8 a = input[0] << 2 | input[1] >> 4;
 uint8 b = input[1] << 4 | input[2] >> 2;
 uint8 c = input[2] << 6 | input[3] >> 0;
strncat(output, decodedstr, oplen-strlen(output));
 return ENCODERBLOCKLEN;
}
int Base64Decode(char *input, uint8 *output, int oplen) {
	int length = 0;
	char *charval = 0;
	char decoderinput[ENCODERLEN + 1] = "";
	int index = 0, asciival = 0, computeval = 0, iplen = 0;
	char encodingtable[TABLELEN + 1] = BASE64CHARSET;	
	 iplen = strlen(input);
	int posactual = 0;
	while (index < iplen){
		asciival = (int)input[index];
		if (asciival == PADDINGCHAR){
			length += decodeblock(decoderinput, output, length);
			break;
		}
		else{
			charval = strchr(encodingtable, asciival);
			if (charval){
				decoderinput[computeval] = charval - encodingtable;
				computeval = (computeval + 1) % 4;
				if(computeval == 0) {
					output[posactual] = decoderinput[0] << 2 | decoderinput[1] >> 4;
					output[posactual+1] = decoderinput[1] << 4 | decoderinput[2] >> 2;
					output[posactual+2] = decoderinput[2] << 6 | decoderinput[3] >> 0;
					decoderinput[0] = decoderinput[1] = decoderinput[2] = decoderinput[3] = 0;
					posactual += 3;
				}
			}
		}
		index++;
	}
	return length;
}

int dectohex (uint8 *input, uint32 *output){
	output [0] = (uint32)((input[0]) << 24 | (input[1]) << 16 | (input[2]) << 8 | input[3]);
	output [1] = (uint32)((input[4]) << 24 | (input[5]) << 16 | (input[6]) << 8 | input[7]);
	output [2] = (uint32)((input[8]) << 24 | (input[9]) << 16 | (input[10]) << 8 | input[11]);
	output [3] = (uint32)((input[12]) << 24 | (input[13]) << 16 | (input[14]) << 8 | input[15]);
	output [4] = (uint32)((input[16]) << 24 | (input[17]) << 16 | (input[18]) << 8 | input[19]);
}
