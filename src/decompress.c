#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "../include/pack_data.h"

static char g_lzss_text[4096+32];
void *inptr = NULL;

void hexPrintf(size_t digits, char *buf) {
	int i;
	for (i = 0; i < digits; i++)
	{
	    if (i > 0) printf(":");
	    printf("%02X", buf[i]);
	}
	printf("\n");
}

// Copied from PM4 DS source code
uint32_t decodeLZ(void *in_src, uint32_t in_size, void **in_ptr)
{
	int i, j, k, r, c, n, f;
	uint32_t size, out_size;
	unsigned int flags;
	void *ptr;
	unsigned char *rd, *wr;

	char debug[32];

	if (!memcmp((void *)in_src, (void *)"LZ08", 4))
	{
		// 9bitƒ^ƒCƒv
		n = 512;
		f = 130;
	} else
	if (!memcmp((void *)in_src, (void *)"LZ12", 4))
	{
		// 12bitƒ^ƒCƒv
		n = 4096;
		f = 18;
	} else {
		//hexPrintf(24, (char *)in_src);
		// LZSS‚¶‚á‚Ë‚¦‚æ
		*in_ptr = in_src;
		return in_size;
	}

	memcpy((void *)&size, (const void *)in_src + 4, 4);
	out_size = size;
	rd = (uint8_t *)(in_src + 8);
	ptr = malloc((unsigned int)size);
	wr = (unsigned char *)ptr;
	in_size -= 8;

	// ƒoƒbƒtƒ@‚ÌƒNƒŠƒA
	for (i=0; i<n-f; i++)
		g_lzss_text[i] = 0;
	// ‘‚«ž‚Ýæ‚ÌˆÊ’u
	r = n - f;
	// ˆ³kƒtƒ‰ƒO
	flags = 0;
	while (size > 0/*TRUE*/)
	{
		// ˆ³kƒtƒ‰ƒO‚ð“Ç‚Ý‚«‚Á‚½‚çAƒtƒ@ƒCƒ‹‚©‚çˆ³kƒtƒ‰ƒO‚ð‚PƒoƒCƒg“Ç‚Ýž‚Þ
		if (((flags >>= 1) & 256) == 0)
		{
			// 1ƒoƒCƒg“Ço‚µB“ü—Í‚ª‚È‚­‚È‚Á‚½‚ç’†’f
			c = *rd;
			rd++;
			///in_size--;
			///if (in_size == 0)
			///	break;
			// ˆ³kƒtƒ‰ƒO‚ÌŽc‚è”‚ð‹L˜^
			flags = (unsigned int)(c | 0xff00);
		}
		// ˆ³kƒf[ƒ^‚©H
		if (flags & 1)
		{
			/* ”ñˆ³kƒf[ƒ^ */
			// ”ñˆ³k•¶Žš“Ço‚µB“ü—Í‚ª‚È‚­‚È‚Á‚½‚ç’†’f
			*wr = *rd;
			rd++;
			///in_size--;
			///if (in_size == 0)
			///	break;
			g_lzss_text[r++] = *wr;		// ƒoƒbƒtƒ@‚Éƒf[ƒ^‚ð‘‚«ž‚Þ
			wr++;
			size--;
			if (size == 0)
				break;
			r &= (n - 1);	// ‘ž‚ÝˆÊ’uXV
		} else {
			/* ˆ³kƒf[ƒ^ */
			// ˆÊ’uŽæ“¾B“ü—Í‚ª‚È‚­‚È‚Á‚½‚ç’†’f
			i = *rd;
			rd++;
			///in_size--;
			///if (in_size == 0)
			///	break;
			// ˆÊ’uAˆê’v’·Žæ“¾B“ü—Í‚ª‚È‚­‚È‚Á‚½‚ç’†’f
			j = *rd;
			rd++;
			///in_size--;
			///if (in_size == 0)
			///	break;
			if (n == 4096)
			{
				i |= ((j & 0xf0) << 4);		// ˆÊ’uŽæ“¾
				j = (j & 0x0f) + 2;			// ˆê’v’·‚ð‹‚ß‚é
			} else {
				// ˆÊ’uŽæ“¾‚Í‚»‚Ì‚Ü‚ñ‚Ü
				i |= ((j & 0x80) << 1);		// ˆÊ’uŽæ“¾
				j = (j & 0x7f) + 2;			// ˆê’v’·‚ð‹‚ß‚é
			}
			// ˆê’v’·•¶Žš•ª‚¾‚¯ƒ‹[ƒv
			for (k=0; k<=j; k++)
			{
				// Žw’è‚³‚ê‚½êŠ‚©‚ç•¶Žš‚ð“Ç‚Ýo‚·
				c = g_lzss_text[(i + k) & (n - 1)];
				// •¶Žšo—Í
				*wr = (unsigned char)c;
				wr++;
				size--;
				if (size == 0)
					break;
				g_lzss_text[r++] = (unsigned char)c;		// ƒoƒbƒtƒ@‚Éƒf[ƒ^‚ð‘‚«ž‚Þ
				r &= (n - 1);		// ‘ž‚ÝˆÊ’uXV
			}
		}
	}

	// Freeing is probably not really necessary
	//free(ptr);

	// freeBDS(in_src);		// ‰ð“€‚µ‚½‚çŒ³‚Í‚¢‚ç‚È‚¢
	*in_ptr = ptr;
	// DC_FlushAll();
	return out_size;
}

unsigned int power(int base, unsigned int exp) {
    int i, result = 1;
    for (i = 0; i < exp; i++)
        result *= base;
    return result;
 }

unsigned int charBufToIntBE(unsigned char *buf, size_t len) {
	unsigned int resultNum;
	int i, k;

	resultNum = 0;
	for (k = 0; k < len; k++) {
		resultNum += abs((unsigned int)(buf[k]) * power(256, k));
	}

	return resultNum;
}

// Define bool type
typedef unsigned int bool;
#define false 0
#define true 1

void mkDecompressedDir(void) {
	system("mkdir decompressed");
	return;
	// if (stat("./decompressed", &st) == -1) {
	// 	mkdir("./decompressed", 0700);
	// }
}

#define fourByteStrSize 5

enum LZType {
	LZ08,
	LZ12,
	LZSS,
	LZUNKNOWN,
};

struct FileResult {
	uint32_t size;
	enum LZType lzType; // NOT ACTUALLY NEEDED since decodeLZ() handles this
	bool isEOF;
};

// char *getBody(FILE *fp, unsigned int len) {
// 	//printf("START BODY\n");
// 	char *mstr = malloc(len + 1);
// 	printf("IS LENGTH %i\n", len + 1);
// 	fgets(mstr, len + 1, fp);
// 	return mstr;
// }

void customFgets(char *dest, size_t len, FILE *fp) {
	for (int i = 0; i < len - 1; i++) {
		dest[i] = (char)fgetc( fp );
	}
	dest[len - 1] = '\0';
}

void main(int argc, char **argv) {
	FILE *fp, *fpOut;
	char sfCount[5];
	bool isDecomp = true, hasOutDir = false;
	struct FileResult fres;

	char *bodyString;

	bodyString = (char *)malloc(1);
	mkDecompressedDir();

	// Argument processing
	if (argc > 0) {
		for (int i = 0; i < argc; i++) {
			if (strncmp(argv[i], "--decompress", 16) == 0) {
				isDecomp = true;
			} else if (strncmp(argv[i], "--compress", 16) == 0) {
				isDecomp = false;
			}
		}
	}

	if (isDecomp) {
		printf("OPERATION = decompress\n");
	} else {
		printf("OPERATION = compress\n");
	}

	unsigned char buf[5];
	unsigned char tokbuf[5];
	unsigned char filePath[1024];
	unsigned char *mstr = malloc(1);
	unsigned char tchar;
	uint32_t outSize;
	bool isChain = false;

	if (isDecomp) {
		// Is pushing files to decompressed (read from data.bin)
		fp = fopen("data.bin", "rb");

		// Goto beginning of file
		fseek((FILE *)fp, 0, SEEK_SET);

		// Iterate over file and find all sigs/bodies
		unsigned int i = 0, cseek = 0;
		uint32_t csize = 0;

		printf("Now parsing files...\n");

		for (int count = 0; count < 6106; count = count + 1) {
			//fseek((FILE *)fp, i, SEEK_CUR);
			if (feof((FILE *)fp)) {
				break;
			}

			// Seeks forward to next item
			fseek((FILE *)fp, g_pack_data_info[count], SEEK_SET);

			customFgets(buf, 5, fp);
			csize = charBufToIntBE(buf, 4);
			//printf("size %i\n", csize);
			//hexPrintf(4, buf);
			// printf("next %s\n", fgets(buf, 5, (FILE *)fp)); break;

			// from base offset
			// +6 = size header
			// +10 = start of file			

			mstr = malloc(csize + 1);
			// char *mstr = getBody((FILE *)fp, csize + 2);
			//printf("New Pointer: %i\n", ftell((FILE *)fp));
			customFgets(mstr, csize + 1, fp);

			outSize = decodeLZ((void *)mstr, csize, &inptr);

			//fwrite(bodyString, sizeof(char), fres.size, (FILE *)fpOut);

			//i = csize + 6;
			sprintf(filePath, "./decompressed/%i.bin", count);
			filePath[1023] = '\0';

			fpOut = fopen(filePath, "wb");
			fseek((FILE *)fpOut, 0, SEEK_SET);
			fwrite((char *)inptr, sizeof(char), outSize, (FILE *)fpOut);
			fclose((FILE *)fpOut);

			free(inptr);
		}

		fclose(fp);
	} else {
		// Is reading files from decompressed
	}

	printf("DONE! SUCCESS!\n");
}
