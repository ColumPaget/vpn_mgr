#include "common.h"
#include "ssh.h"

STREAM *Terminal=NULL;
int GlobalFlags=0;

void SignalHandler(int sig)
{
    switch(sig)
    {
    case SIGKILL:
    case SIGTERM:
    case SIGINT:
        GlobalFlags |= FLAG_EXIT;
        SSHVpnTerminate();
        //	kill(0-getpid(), SIGINT);
        break;
    }

}


char *ReadFile(char *RetStr, const char *Path)
{
    STREAM *S;

    RetStr=CopyStr(RetStr, "");
    if (! StrValid(Path)) return(RetStr);

    S=STREAMOpen(Path, "r");
    if (S)
    {
        RetStr=STREAMReadDocument(RetStr, S);
        STREAMClose(S);
    }

    return(RetStr);
}
