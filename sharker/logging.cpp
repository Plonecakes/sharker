#include "logging.h"

static char* const LOG_FILE_NAME = "mod_sharker.log";
static const char LOG_BEGIN[] = "\xef\xbb\xbfsharker version 3.0p by Plonecakes\r\n";

// TODO: If the file is busy, it must wait to write.

FILE *BeginLog() {
#pragma warning(push)
#pragma warning(disable: 4996)
	FILE *log_file = fopen(LOG_FILE_NAME, "ab+");
	fseek(log_file, 0, SEEK_END);
#pragma warning(pop)
	if (log_file != NULL && ftell(log_file) == 0) {
		fwrite(LOG_BEGIN, sizeof(char), sizeof(LOG_BEGIN) / sizeof(char)-1, log_file);
	}
	return log_file;
}

inline void CloseLog(FILE *log_file) {
	fclose(log_file);
}

void LogMessage(const char *txt, ...) {
	FILE *log_file = BeginLog();
	if (log_file) {
		va_list args;
		va_start(args, txt);
		vfprintf(log_file, txt, args);
		fwrite("\r\n", sizeof(char), 2, log_file);
		va_end(args);
	}
	CloseLog(log_file);
}

void LogMessageW(const WCHAR *txt, ...) {
	FILE *log_file = BeginLog();
	if (log_file) {
		va_list args;
		va_start(args, txt);
		int len = wcslen(txt) * 2 + MAX_PATH + 1000;
		char *buff = new char[len];
		WideCharToMultiByte(CP_UTF8, 0, txt, -1, buff, len, NULL, NULL);
		vfprintf(log_file, buff, args);
		fwrite("\r\n", sizeof(char), 2, log_file);
		delete[] buff;
		va_end(args);
	}
	CloseLog(log_file);
}