#ifndef VPN_MGR_COMMON_H
#define VPN_MGR_COMMON_H

#include "libUseful-5/libUseful.h"

#define FLAG_EXIT            1
#define FLAG_SU              2
#define FLAG_LOCAL_SU        2
#define FLAG_SUDO            4
#define FLAG_LOCAL_SUDO      4
#define FLAG_REMOTE_SU       8
#define FLAG_REMOTE_SUDO    16
#define FLAG_NODNS          32
#define FLAG_DEBUG        2048
#define FLAG_VERBOSE      4096

extern STREAM *Terminal;
extern int GlobalFlags;

void SignalHandler(int sig);
char *ReadFile(char *RetStr, const char *Path);

#endif
