#pragma once

#include "stdafx.h"

#define PIDVE "[%05i.e] "
#define PIDVD "[%05i.d] "

void LogMessage(const char *txt, ...);
void LogMessageW(const WCHAR *txt, ...);

// This allows compile-time debug logging. Just switch it when you need!
// The first performs the log, and the second removes them from the code.
//#define DebugLog LogMessage
#define DebugLog(a, ...)