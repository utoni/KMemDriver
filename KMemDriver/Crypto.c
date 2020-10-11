#include "Crypto.h"

struct crypt_data {
	UINT64 key;
	UINT8 crypted;
	UINT32 marker;
};

#define MAX_CRYPTED_FUNCTIONS 64
static struct crypt_data data[MAX_CRYPTED_FUNCTIONS];
static size_t data_used = 0;

void crypt_fn(void)
{
}