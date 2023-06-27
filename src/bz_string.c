#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef struct bz_string {
	u_char *ptr;
	size_t len;
	size_t capacity;
} bz_str_t;

bz_str_t bz_string(const char *str) {

	bz_str_t string;

	string.len = strlen(str);
	string.capacity = 2 * string.len;

	string.ptr = malloc(sizeof(*str) * string.capacity);

	memcpy(string.ptr, str, string.len);

	*(string.ptr + string.len) = '\0';

	return string;
}

int bz_str_append(bz_str_t *str, const char *c_str) {

	assert(c_str != NULL && "Null string passed to append.");

	if (str->len + strlen(c_str) + 1 > str->capacity) {

		str->ptr = realloc(str->ptr, str->capacity * 2);
		if (!str->ptr) {
			fprintf(stderr, "couldn't allocate more memory.\n");
			return -1;
		}
		str->capacity *= 2;
	}

	memcpy(str->ptr + str->len, c_str, strlen(c_str));
	str->len += strlen(c_str) + 1;

	*(str->ptr + str->len) = '\0';

	return 0;
}


