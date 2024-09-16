#include "run_command.h"
#include <wait.h>

static ListNode *FoundCommands=NULL;
static ListNode *Passwords=NULL;
static int RunCommandPasswordAttemptCount=0;

void FindCommands(const char *Commands)
{
    char *Tempstr=NULL, *Item=NULL, *Path=NULL;
    const char *ptr;

    Tempstr=CopyStr(Tempstr, getenv("PATH"));
    Tempstr=CatStr(Tempstr, ":/sbin:/usr/sbin:/usr/local/sbin");

    if (! FoundCommands) FoundCommands=ListCreate();

    ptr=GetToken(Commands, ",", &Item, 0);
    while (ptr)
    {
        Path=FindFileInPath(Path, Item, Tempstr);
        if (StrValid(Path)) SetVar(FoundCommands, Item, Path);
        ptr=GetToken(ptr, ",", &Item, 0);
    }

    Destroy(Tempstr);
    Destroy(Item);
    Destroy(Path);
}

const char *GetCommandPath(const char *Command)
{
    if (! FoundCommands) return("");
    return(GetVar(FoundCommands, Command));
}


int RunCommandPasswordTransact(const char *PasswordDomain, const char *PasswordProgram, STREAM *S)
{
    char *Prompt=NULL, *PasswordType=NULL, *Password=NULL;
    int RetVal=RUN_CMD_OKAY;

    PasswordType=MCopyStr(PasswordType, PasswordDomain, " ", PasswordProgram, NULL);
    Password=CopyStr(Password, GetVar(Passwords, PasswordType));
    if (! StrValid(Password))
    {
        printf("\n");
        fflush(NULL);
        Prompt=MCopyStr(Prompt, "~ePassword for ~y", PasswordDomain, "~0 ~e", PasswordProgram, ":~0 ", NULL);
        Password=TerminalReadPrompt(Password, Prompt, TERM_SHOWTEXTSTARS, Terminal);
        printf("\n");

        if (StrValid(Password))
        {
            if (! Passwords) Passwords=ListCreate();
            SetVar(Passwords, PasswordType, Password);
        }
        else RetVal=RUN_CMD_ABORT;
    }
    RunCommandPasswordAttemptCount++;

    if (GlobalFlags & FLAG_DEBUG) printf("SEND PASSWORD: [%s]\n", Password);
    STREAMWriteLine(Password, S);
    STREAMWriteLine("\n", S);
    STREAMFlush(S);

    Destroy(PasswordType);
    Destroy(Password);
    Destroy(Prompt);

    return(RetVal);
}



char *RunCmdFormat(char *RetStr, const char *Cmd, const char *Args, int Flags)
{
    const char *PtermArg="", *Prefix="";

    if (Flags & CMD_SSH) Prefix="";
    else Prefix="cmd:";


    // vpn-mgr-sudo-done and vpn-mgr-su-done are mostly useful for remote su/sudo.
    // The only way for us to know that an su/sudo has finished/failed is by printing an 'su-done' message

    if (Flags & CMD_SUDO)
    {
        if (StrValid(GetCommandPath("sudo"))) RetStr=MCopyStr(RetStr, Prefix, "sudo -S /bin/sh -c 'echo vpn-mgr-cmd-run;", Cmd, " ", Args, "'; echo vpn-mgr-sudo-done",  NULL);
    }
    else if (Flags & CMD_SU)
    {
        if (StrValid(GetCommandPath("su"))) RetStr=MCopyStr(RetStr, Prefix, "su ", PtermArg, " -c 'echo vpn-mgr-cmd-run; ", Cmd, " ", Args, "'; echo vpn-mgr-su-done", NULL);
    }
    else RetStr=MCopyStr(RetStr, Prefix, Cmd, " ", Args, NULL);

    return(RetStr);
}



char *RunCmdSetup(char *RetStr, const char *Cmd, int Flags)
{
    char *Token=NULL;
    const char *ptr, *path;

    RetStr=CopyStr(RetStr, "");
    ptr=GetToken(Cmd, "\\S", &Token, 0);
		if (*Token =='/') path=Token; //we have a full path
    else path=GetCommandPath(Token);
    if (! StrValid(path))
    {
        Destroy(Token);
        Destroy(RetStr);
        return(NULL);
    }

    RetStr=RunCmdFormat(RetStr, path, ptr, Flags);
    Destroy(Token);

    return(RetStr);
}





int RunCommandCleanUp(STREAM *S, int Flags)
{
    const char *ptr;
    pid_t pid=-1;
    int status;

    ptr=STREAMGetValue(S, "PeerPID");
    if (StrValid(ptr))
    {
        pid=atoi(ptr);
        if ((pid > -1) && (Flags & CMD_KILL)) kill(0-pid, SIGTERM);
        waitpid(pid, &status, 0);
    }
    STREAMClose(S);

    if (status == 0) return(RUN_CMD_OKAY);
    return(RUN_CMD_FAIL);
}



int RunCommandReadLoop(STREAM *S, const char *PasswordDomain, const char *PasswordProgram)
{
    char *Tempstr=NULL, *CheckStr=NULL;
    int result=RUN_CMD_BEGIN;

    Tempstr=STREAMReadToMultiTerminator(Tempstr, S, ":\r\n");
    while (Tempstr)
    {
        if (StrValid(Tempstr))
        {
            CheckStr=CopyStr(CheckStr, Tempstr);
            StripLeadingWhitespace(CheckStr);
            StripTrailingWhitespace(CheckStr);

            if (GlobalFlags & FLAG_DEBUG) fprintf(stderr, "Command Output: %s\n", CheckStr);

            if (strncmp(CheckStr, "[sudo] password for", 19) ==0)
            {
                result=RunCommandPasswordTransact(PasswordDomain, PasswordProgram, S);
            }
            else if (strncmp(CheckStr, "Password:", 9) ==0)
            {
                result=RunCommandPasswordTransact(PasswordDomain, PasswordProgram, S);
            }
            else if (strncmp(CheckStr, "Sorry", 5) ==0)
            {
                result=RUN_CMD_AUTH_FAIL;
                break;
            }
            else if (strcmp(CheckStr, "Authentication failure") ==0)
            {
                result=RUN_CMD_AUTH_FAIL;
                break;
            }
            else if (strcmp(CheckStr, "vpn-mgr-cmd-run") == 0)
            {
                result=RUN_CMD_OKAY;
                break;
            }
            // these are mostly useful for remote su/sudo.
            // The only way for us to know that an su/sudo has finished/failed is by printing an 'su-done' message
            else if (strcmp(CheckStr, "vpn-mgr-sudo-done") == 0) break;
            else if (strcmp(CheckStr, "vpn-mgr-su-done") == 0) break;
            // Don't both to print empty lines, but print 'Tempstr' rather than 'CheckStr'
            // in order to preserve real formatting of lines
            else if (StrValid(CheckStr)) printf("   %s", Tempstr);

            //if result has changed, then we either failed or succeeded
            if ( (result != RUN_CMD_BEGIN) && (result != RUN_CMD_OKAY) ) break;
        }

        if (GlobalFlags & FLAG_EXIT) break;
        Tempstr=STREAMReadToMultiTerminator(Tempstr, S, ":\r\n");
    }

    Destroy(Tempstr);
    Destroy(CheckStr);

    return(result);
}




static STREAM *RunCommandOpenTry(const char *Cmd, int Flags)
{
    char *FullCmd=NULL, *Tempstr=NULL;
    STREAM *S=NULL;
    int result;

    if (getuid() == 0) Flags &= ~(CMD_SUDO | CMD_SU);
    FullCmd=RunCmdSetup(FullCmd, Cmd, Flags);


    if (StrValid(FullCmd))
    {
        if ( (Flags & CMD_VERBOSE) || (GlobalFlags & (FLAG_VERBOSE | FLAG_DEBUG)) ) TerminalPrint(Terminal, "~yRUN CMD:~0 [%s]\n", FullCmd);
        Tempstr=MCopyStr(Tempstr, "rw pty setsid");
        if (! (Flags & CMD_NO_STDERR)) Tempstr=CatStr(Tempstr, " +stderr");

        S=STREAMOpen(FullCmd, Tempstr);
        if (S)
        {
            if (Flags & (CMD_SUDO | CMD_SU))
            {
                result=RunCommandReadLoop(S, "Local", "su/sudo");

                switch (result)
                {
                case RUN_CMD_AUTH_FAIL:
                    TerminalPrint(Terminal, "~r~eERROR: Authentication Failure~0\n");
                    STREAMClose(S);
                    S=NULL;
                    break;

                case RUN_CMD_ABORT:
                    TerminalPrint(Terminal, "~r~eERROR: User Aborted command. Exiting...~0\n");
                    STREAMClose(S);
                    TerminalReset(Terminal);
                    exit(1);
                    break;
                }
            }
        }
        else TerminalPrint(Terminal, "~r~eERROR: Failed to run command: [%s]~0\n", FullCmd);
    }
    else
    {
        GetToken(Cmd, " ", &Tempstr, 0);
        TerminalPrint(Terminal, "~r~eERROR: cmd '%s' unknown~0\n", Tempstr);
    }

    Destroy(Tempstr);
    Destroy(FullCmd);

    return(S);
}



STREAM *RunCommandOpen(const char *Cmd, int CmdFlags)
{
    STREAM *S=NULL;

    if ( (CmdFlags & CMD_ASROOT) && (getuid() !=0) )
    {
        if (GlobalFlags & FLAG_SU) CmdFlags |= CMD_SU;
        if (GlobalFlags & FLAG_SUDO) CmdFlags |= CMD_SUDO;
    }


    setenv("LC_ALL", "C", TRUE);
    setenv("LANG", "C", TRUE);
    if (CmdFlags & CMD_SUDO) S=RunCommandOpenTry(Cmd, CmdFlags);

    if (S==NULL)
    {
        if (CmdFlags & CMD_SUDO)
        {
            TerminalPrint(Terminal, "~y~e'sudo' invocation failed, attempting 'su'~0\n");
            GlobalFlags &= ~CMD_SUDO;
            CmdFlags &= ~CMD_SUDO;
        }
        S=RunCommandOpenTry(Cmd, CmdFlags);
    }

    return(S);
}


int RunCommand(const char *Cmd, int Flags)
{
    int result=-1;
    char *Tempstr=NULL;
    STREAM *S;

    S=RunCommandOpen(Cmd, Flags);
    if (S)
    {
        Tempstr=STREAMReadLine(Tempstr, S);
        while (Tempstr)
        {

            if (GlobalFlags & FLAG_VERBOSE)
            {
                //we don't ever really want to print out the messages we send to say that
                //an su/sudo attempt is over, they are just confusing to see
                if (strncmp(Tempstr, "vpn-mgr-", 8) ==0) /*do nothing */ ;
                else printf("%s", Tempstr);
            }

            if (GlobalFlags & FLAG_EXIT) break;
            Tempstr=STREAMReadLine(Tempstr, S);
        }
        result=RunCommandCleanUp(S, Flags);
    }
    else
    {
        TerminalReset(Terminal);
        _exit(1);
    }

    Destroy(Tempstr);

    return(result);
}

