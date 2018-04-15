#ifndef _PVT_LIBPARSE_H_
#define _PVT_LIBPARSE_H_

/*----------------------------------------------------------------------------*/

void exit_usage(void);
void exit_missing_arg(const char *optname);
void exit_unknown_arg(const char *optname);
void exit_help(void);
void exit_version(void);
void exit_badvalue(const char *shortarg, const char *longarg,
                   const char *value);

/*----------------------------------------------------------------------------*/

#endif
