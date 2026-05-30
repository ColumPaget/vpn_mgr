#include "vpn_config.h"
#include <glob.h>


static char *ReadFileConf(char *RetStr, const char *Value)
{
RetStr=CopyStr(RetStr, "");

if (StrValid(Value))
{
if (strncmp(Value, "~/", 2)==0) RetStr=MCopyStr(RetStr, GetCurrUserHomeDir(), Value+1, NULL);
else RetStr=CopyStr(RetStr, Value);
}

return(RetStr);
}


void ReadConfigLine(TVpn *Vpn, const char *Line)
{
    char *Key=NULL, *Value=NULL;
    const char *ptr;

    ptr=GetNameValuePair(Line, " ", "=", &Key, &Value);
    while (ptr)
    {
        if (strcasecmp(Key, "Server")==0) Vpn->Server=CopyStr(Vpn->Server, Value);

        if (strcasecmp(Key, "ClientCert")==0) Vpn->ClientCert=ReadFileConf(Vpn->ClientCert, Value);
        if (strcasecmp(Key, "ClientKey")==0) Vpn->ClientKey=ReadFileConf(Vpn->ClientKey, Value);
        if (strcasecmp(Key, "VerifyCert")==0) Vpn->VerifyCert=ReadFileConf(Vpn->VerifyCert, Value);
        if (strcasecmp(Key, "ServerCert")==0) Vpn->VerifyCert=ReadFileConf(Vpn->VerifyCert, Value);
        if (strcasecmp(Key, "ConfFile")==0) Vpn->ConfFile=ReadFileConf(Vpn->ConfFile, Value);
        if (strcasecmp(Key, "UpFile")==0) Vpn->UpFile=ReadFileConf(Vpn->UpFile, Value);
        if (strcasecmp(Key, "DownFile")==0) Vpn->DownFile=ReadFileConf(Vpn->DownFile, Value);

        if (strcasecmp(Key, "LocalAddress")==0) Vpn->LocalAddress=CopyStr(Vpn->LocalAddress, Value);
        if (strcasecmp(Key, "AllowedIPs")==0) Vpn->AllowedIPs=CommaList(Vpn->AllowedIPs, Value);
        if (strcasecmp(Key, "RemoteDev")==0) Vpn->RemoteDev=CopyStr(Vpn->RemoteDev, Value);
        if (strcasecmp(Key, "ClientID")==0) Vpn->ClientID=CopyStr(Vpn->ClientID, Value);
        if (strcasecmp(Key, "UserName")==0) Vpn->UserName=CopyStr(Vpn->UserName, Value);
        if (strcasecmp(Key, "Password")==0) Vpn->Password=CopyStr(Vpn->Password, Value);
        if (strcasecmp(Key, "RemoteSearchPath")==0) Vpn->RemoteSearchPath=CopyStr(Vpn->RemoteSearchPath, Value);
        if (strcasecmp(Key, "DNS")==0) Vpn->DNS=CopyStr(Vpn->DNS, Value);
        if (strcasecmp(Key, "Timeout")==0) Vpn->Timeout=ParseDuration(Value);
        if (strcasecmp(Key, "Protocol")==0) Vpn->Flags |= VPN_TCP;

        if (strcasecmp(Key, "RemoteSU")==0)
        {
            //clear flags
            GlobalFlags &= ~(FLAG_REMOTE_SU |  FLAG_REMOTE_SUDO);

            //this is redunant, just included so that there's code to handle 'none'
            if (strcmp(Value, "none")==0) GlobalFlags &= ~(FLAG_REMOTE_SU |  FLAG_REMOTE_SUDO);
            else if (strcmp(Value, "su")==0) GlobalFlags |= FLAG_REMOTE_SU;
            else if (strcmp(Value, "sudo")==0) GlobalFlags |= FLAG_REMOTE_SUDO;
        }

        if (strcasecmp(Key, "LocalSU")==0)
        {
            //clear flags
            GlobalFlags &= ~(FLAG_LOCAL_SU |  FLAG_LOCAL_SUDO);

            //this is redunant, just included so that there's code to handle 'none'
            if (strcmp(Value, "none")==0) GlobalFlags &= ~(FLAG_LOCAL_SU |  FLAG_LOCAL_SUDO);
            else if (strcmp(Value, "su")==0) GlobalFlags |= FLAG_LOCAL_SU;
            else if (strcmp(Value, "sudo")==0) GlobalFlags |= FLAG_LOCAL_SUDO;
        }

        ptr=GetNameValuePair(ptr, " ", "=", &Key, &Value);
    }

    Destroy(Key);
    Destroy(Value);
}


void ReadConfig(TVpn *Vpn)
{
    char *Tempstr=NULL, *Key=NULL, *Value=NULL;
    const char *ptr;
    STREAM *S;

    Tempstr=MCopyStr(Tempstr, GetCurrUserHomeDir(), "/.config/vpn_mgr/", Vpn->Name, "/", Vpn->Name, ".conf", NULL);

    S=STREAMOpen(Tempstr, "r");
    if (S)
    {
        Tempstr=STREAMReadLine(Tempstr, S);
        while (Tempstr)
        {
            StripTrailingWhitespace(Tempstr);
            ReadConfigLine(Vpn, Tempstr);
            Tempstr=STREAMReadLine(Tempstr, S);
        }

        STREAMClose(S);
    }

    Destroy(Tempstr);
    Destroy(Value);
    Destroy(Key);
}



void SaveFilePath(STREAM *S, const char *Name, const char *Path, const char *Home)
{
char *Tempstr=NULL;
int hlen;

        if (StrValid(Path))
        {
            Tempstr=MCopyStr(Tempstr, Name, "=", NULL);
	    hlen=StrLen(Home);
	    if (strncmp(Path, Home, hlen)==0) Tempstr=MCatStr(Tempstr, "~/", Path + hlen, "\n", NULL);
            else Tempstr=MCatStr(Tempstr, Path, "\n", NULL);
            STREAMWriteLine(Tempstr, S);
        }

Destroy(Tempstr);
}



void WriteConfig(TVpn *Vpn)
{
    char *Tempstr=NULL, *Home=NULL;
    STREAM *S;


    Home=MCopyStr(Home, GetCurrUserHomeDir(), "/", NULL);
    Tempstr=MCopyStr(Tempstr, Home, "/.config/vpn_mgr/", Vpn->Name, "/", Vpn->Name, ".conf", NULL);
    MakeDirPath(Tempstr, 0700);


    S=STREAMOpen(Tempstr, "w");
    if (S)
    {
        Tempstr=MCopyStr(Tempstr, "Server=", Vpn->Server, "\n", NULL);
        STREAMWriteLine(Tempstr, S);

        SaveFilePath(S, "ClientCert", Vpn->ClientCert, Home);
        SaveFilePath(S, "ClientKey", Vpn->ClientKey, Home);
        SaveFilePath(S, "VerifyCert", Vpn->VerifyCert, Home);
        SaveFilePath(S, "ConfFile", Vpn->ConfFile, Home);
        SaveFilePath(S, "UpFile", Vpn->UpFile, Home);
        SaveFilePath(S, "DownFile", Vpn->DownFile, Home);


        if (StrValid(Vpn->LocalAddress))
        {
            Tempstr=MCopyStr(Tempstr, "LocalAddress=", Vpn->LocalAddress, "\n", NULL);
            STREAMWriteLine(Tempstr, S);
        }

        if (StrValid(Vpn->AllowedIPs))
        {
            Tempstr=MCopyStr(Tempstr, "AllowedIPs=", Vpn->AllowedIPs, "\n", NULL);
            STREAMWriteLine(Tempstr, S);
        }

        if (StrValid(Vpn->RemoteDev))
        {
            Tempstr=MCopyStr(Tempstr, "RemoteDev=", Vpn->RemoteDev, "\n", NULL);
            STREAMWriteLine(Tempstr, S);
        }

        if (StrValid(Vpn->DNS))
        {
            Tempstr=MCopyStr(Tempstr, "DNS=", Vpn->DNS, "\n", NULL);
            STREAMWriteLine(Tempstr, S);
        }

        if (StrValid(Vpn->ClientID))
        {
            Tempstr=MCopyStr(Tempstr, "ClientID=", Vpn->ClientID, "\n", NULL);
            STREAMWriteLine(Tempstr, S);
        }

        if (StrValid(Vpn->UserName))
        {
            Tempstr=MCopyStr(Tempstr, "UserName=", Vpn->UserName, "\n", NULL);
            STREAMWriteLine(Tempstr, S);
        }

        if (StrValid(Vpn->Password))
        {
            Tempstr=MCopyStr(Tempstr, "Password=", Vpn->Password, "\n", NULL);
            STREAMWriteLine(Tempstr, S);
        }

        if (StrValid(Vpn->RemoteSearchPath))
        {
            Tempstr=MCopyStr(Tempstr, "RemoteSearchPath=", Vpn->RemoteSearchPath, "\n", NULL);
            STREAMWriteLine(Tempstr, S);
        }

        if (Vpn->Timeout > 0)
        {
            Tempstr=FormatStr(Tempstr, "Timeout=%ld", Vpn->Timeout, "\n", NULL);
            STREAMWriteLine(Tempstr, S);
        }


        if (Vpn->Flags & VPN_TCP) STREAMWriteLine("Protocol=tcp\n", S);

        switch (GlobalFlags & (FLAG_LOCAL_SU | FLAG_LOCAL_SUDO))
        {
        case 0:
            STREAMWriteLine("LocalSU=none\n", S);
            break;
        case FLAG_LOCAL_SU:
            STREAMWriteLine("LocalSU=su\n", S);
            break;
        case FLAG_LOCAL_SUDO:
            STREAMWriteLine("LocalSU=sudo\n", S);
            break;
        }

        switch (GlobalFlags & (FLAG_REMOTE_SU | FLAG_REMOTE_SUDO))
        {
        case 0:
            STREAMWriteLine("RemoteSU=none\n", S);
            break;
        case FLAG_REMOTE_SU:
            STREAMWriteLine("RemoteSU=su\n", S);
            break;
        case FLAG_REMOTE_SUDO:
            STREAMWriteLine("RemoteSU=sudo\n", S);
            break;
        }

        STREAMClose(S);
    }

    Destroy(Tempstr);
    Destroy(Home);
}


static int PathValid(const char *Path)
{
    if (! StrValid(Path)) return(FALSE);

    if (access(Path, F_OK) !=0)
    {
        printf("ERROR: File [%s] does not exist\n", Path);
        return(FALSE);
    }

    if (access(Path, R_OK) !=0)
    {
        printf("ERROR: File [%s] is not readable\n", Path);
        return(FALSE);
    }

    return(TRUE);
}


void AddConfig(TVpn *Vpn)
{
    char *Dir=NULL, *Tempstr=NULL;


    Dir=MCopyStr(Dir, GetCurrUserHomeDir(), "/.config/vpn_mgr/", Vpn->Name, "/", NULL);
    MakeDirPath(Dir, 0700);

    if (PathValid(Vpn->ClientCert))
    {
        Tempstr=MCopyStr(Tempstr, Dir, Vpn->Name, ".crt", NULL);
        FileCopy(Vpn->ClientCert, Tempstr);
        chmod(Tempstr, 0600);
        Vpn->ClientCert=CopyStr(Vpn->ClientCert, Tempstr);
    }

    if (PathValid(Vpn->ClientKey))
    {
        Tempstr=MCopyStr(Tempstr, Dir, Vpn->Name, ".key", NULL);
        FileCopy(Vpn->ClientKey, Tempstr);
        chmod(Tempstr, 0600);
        Vpn->ClientKey=CopyStr(Vpn->ClientKey, Tempstr);
    }

    if (PathValid(Vpn->VerifyCert))
    {
        Tempstr=MCopyStr(Tempstr, Dir, Vpn->Name, ".server", NULL);
        FileCopy(Vpn->VerifyCert, Tempstr);
        chmod(Tempstr, 0600);
        Vpn->VerifyCert=CopyStr(Vpn->VerifyCert, Tempstr);
    }

    WriteConfig(Vpn);

    Destroy(Tempstr);
    Destroy(Dir);
}

void ListConfigs(TVpn *Info)
{
    char *Tempstr=NULL;
    glob_t Glob;
    TVpn *Vpn;
    int i;

    Tempstr=MCopyStr(Tempstr, GetCurrUserHomeDir(), "/.config/vpn_mgr/*/*.conf", NULL);
    glob(Tempstr, 0, 0, &Glob);
    for (i=0; i < Glob.gl_pathc; i++)
    {
        Tempstr=CopyStr(Tempstr, GetBasename(Glob.gl_pathv[i]));
        StrRTruncChar(Tempstr, '.');
        Vpn=VpnCtxCreate("", Tempstr, "");
        ReadConfig(Vpn);
        printf("%s %s\n", Vpn->Name, Vpn->Server);
    }

    globfree(&Glob);
    Destroy(Tempstr);
}


void DeleteConfig(TVpn *Vpn)
{
    char *Dir=NULL;

    Dir=MCopyStr(Dir, GetCurrUserHomeDir(), "/.config/vpn_mgr/", Vpn->Name, "/", NULL);
    FileSystemRmDir(Dir);

    Destroy(Dir);
}

