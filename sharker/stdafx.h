// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

// TODO: reference additional headers your program requires here

#define INI_FILE_NAME  ".\\sharker.ini"
#define INI_HEADER     "Options"
#define INI_LOADINI    "LoadINI"
#define INI_LOADDLL    "LoadDLL"
#define INI_LOADFOLDER "LoadFolder"
#define INI_PROXYDLL   "ProxyDLL"
#define INI_PROXYEXE   "ProxyEXE"