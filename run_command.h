#ifndef VPN_MGR_RUN_COMMAND_H
#define VPN_MGR_RUN_COMMAND_H

#include "common.h"

#define CMD_ASROOT        1
#define CMD_SU            2
#define CMD_SUDO          4
#define CMD_SSH           8
#define CMD_NOSYNC       16
#define CMD_NO_STDERR    32
#define CMD_KILL       4096
#define CMD_VERBOSE   65536

#define RUN_CMD_BEGIN     0
#define RUN_CMD_OKAY      1
#define RUN_CMD_UNKNOWN   2
#define RUN_CMD_AUTH_FAIL 3
#define RUN_CMD_FAIL      4
#define RUN_CMD_ABORT     5


void FindCommands(const char *Commands);
const char *GetCommandPath(const char *Command);

char *RunCmdFormat(char *RetStr, const char *Cmd, const char *Args, int Flags);
int RunCommandReadLoop(STREAM *S, const char *PasswordDomain, const char *PasswordProgram);
STREAM *RunCommandOpen(const char *Cmd, int Flags);
int RunCommandCleanUp(STREAM *S, int Flags);
int RunCommand(const char *Cmd, int AsRoot);

#endif
