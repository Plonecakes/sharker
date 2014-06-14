#include "stdafx.h"
#include "logging.h"

static char DLL_NAME[] = "sharker.dll";

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// Load EXE Proxy and pass arguments.
	char myname[MAX_PATH];
	if (GetModuleFileName(NULL, myname, sizeof(myname)) == 0) {
		// Log failure.
		LogMessage(PIDVE "Could not resolve own name.", GetCurrentProcessId());
		return 1;
	}

	if (_stricmp(myname + strlen(myname) - 8, "jaws.exe") == 0) {
		// Running a test instead...
		LoadLibrary(DLL_NAME);
		while (true) {
			Sleep(1000);
		}
		return 0;
	}

	return 0;
}