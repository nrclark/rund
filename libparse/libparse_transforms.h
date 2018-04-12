#ifndef _LIBPARSE_TRANSFORMS_H_
#define _LIBPARSE_TRANSFORMS_H_

/* The functions listed here are used to process input arguments. Each
 * one reads a string from *input, converts it, and stores the result
 * in *output. Functions return 0 in the event of a success, and something
 * else otherwise. */

int arg_int64(const char *input, void *output);
int arg_uint64(const char *input, void *output);
int arg_int(const char *input, void *output);
int arg_uid(const char *input, void *output);
int arg_gid(const char *input, void *output);

#endif
