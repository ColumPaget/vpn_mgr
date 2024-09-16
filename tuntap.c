#include "tuntap.h"
#include "run_command.h"
#include "net.h"

#define TUN_PATH "/dev/net/tun"

void TunModprobe()
{
    TerminalPrint(Terminal, "~e~cTUN:~0 %s does not exist. Trying 'modprobe tun' in case kernel module not loaded \n", TUN_PATH);
    RunCommand("modprobe tun", CMD_ASROOT);
    RunCommand("chmod a+rw /dev/net/tun", CMD_ASROOT);
}

int TunAvailable()
{
    int result=FALSE;

    if (access(TUN_PATH, F_OK) !=0)
    {
        TunModprobe();
    }

    RunCommand("chmod a+rw /dev/net/tun", CMD_ASROOT);

    if (access(TUN_PATH, F_OK) !=0) TerminalPrint(Terminal, "~e~cTUN:~0~e~rERROR~0. %s does not seem to exist\n", TUN_PATH);
    else if (access(TUN_PATH, W_OK) !=0) TerminalPrint(Terminal, "~e~cTUN:~0~e~rERROR~0. %s not writeable\n", TUN_PATH);
    else if (access(TUN_PATH, R_OK) !=0) TerminalPrint(Terminal, "~e~cTUN:~0~e~rERROR~0. %s not readable\n", TUN_PATH);
    else result=TRUE;

    return(result);
}



int TunSetup(const char *Type, const char *Name, const char *Address)
{
    char *Tempstr=NULL;
    int status=RUN_CMD_FAIL;

    if (! NetDevExists(Name))
    {
        TunAvailable();

        TerminalPrint(Terminal, "~e~cTUN:~0 create local tun device [~e%s~0]\n", Name);
        if (strcmp(Type, "tun") == 0) Tempstr=MCopyStr(Tempstr, "ip tuntap add dev ", Name, " mode tun", NULL);
        else Tempstr=MCopyStr(Tempstr, "ip link add dev ", Name, " type ", Type, NULL);

        status=RunCommand(Tempstr, CMD_ASROOT);
    }
    else status=RUN_CMD_OKAY;



    if (status == RUN_CMD_OKAY)
    {
        if (StrValid(Address))
        {
            Tempstr=MCopyStr(Tempstr, "ip address add dev ", Name, " ", Address, NULL);
            RunCommand(Tempstr, CMD_ASROOT);
        }

        TerminalPrint(Terminal, "~e~cTUN:~0 activate local tun device [~e%s~0]\n", Name);
        Tempstr=MCopyStr(Tempstr, "ip link set up dev ", Name, NULL);
        RunCommand(Tempstr, CMD_ASROOT);


        if (! NetDevExists(Name)) status=RUN_CMD_FAIL;
    }
    else TerminalPrint(Terminal, "~e~cTUN:~0~e~rFATAL~0. Failed to create interface [~e%s~0]\n", Name);

    Destroy(Tempstr);

    return(status);
}


void TunShutdown(const char *Dev)
{
    char *Tempstr=NULL;

		TerminalPrint(Terminal, "~e~cTUN:~0 shutdown local tun devices [~e%s~0]\n", Dev);
    Tempstr=MCopyStr(Tempstr, "ip link set dev ", Dev, " down ", NULL);
    RunCommand(Tempstr, CMD_ASROOT);

    Tempstr=MCopyStr(Tempstr, "ip link delete dev ", Dev, NULL);
    RunCommand(Tempstr, CMD_ASROOT);

    Destroy(Tempstr);
}
