#include "Crypto.h"

#include <stdarg.h>

struct crypt_data {
	UINT64 key;
	UINT8 crypted;
	UINT8 used;
};

static struct crypt_data * data = NULL;
static size_t data_used = 0;

void CryptoInit(PVOID fn, ...)
{
	va_list ap;

	va_start(ap, fn);
	PVOID f;
	while ((f = va_arg(ap, PVOID)) != NULL)
	{

	}
	va_end(ap);
}

void CryptoDo(PVOID fn)
{
	UNREFERENCED_PARAMETER(fn);
}