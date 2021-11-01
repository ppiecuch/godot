/*************************************************************************/
/*  clw.c                                                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

void __fatal(const char *msg, ...) {
	va_list ap;
	fprintf(stderr, "clw: fatal: ");
	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);
	fprintf(stderr, "\n");
#ifdef _WIN32
	// On Windows, some tools may inject extra threads.
	// exit() may block on locks held by those threads, so forcibly exit.
	fflush(stderr);
	fflush(stdout);
	ExitProcess(1);
#else
	exit(1);
#endif
}

typedef struct {
	char *string;
	int size;
} string;

string string_new() {
	string s;
	s.string = NULL;
	s.size = 0;
	return s;
}
void string_free(string *s) {
	free(s->string);
}
string string_init(const char *cstr) {
	string s;
	s.string = _strdup(cstr);
	s.size = strlen(cstr);
	return s;
}
void string_append(string *s, const char *data, size_t data_len) {
	int new_size = s->size + data_len;
	s->string = realloc(s->string, new_size + 1);
	if (!s->string) {
		__fatal("failed to realloc string");
	}
	memcpy(s->string + s->size, data, data_len);
	s->string[new_size] = 0;
	s->size = new_size;
}
void string_cappend(string *s, const char *cstr) {
	string_append(s, cstr, strlen(cstr));
}
char *string_cstr(string s) {
	return s.string;
}
int string_size(string s) {
	return s.size;
}

string GetLastErrorString() {
	DWORD err = GetLastError();

	char *msg_buf = NULL;
	FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(char *)&msg_buf,
			0,
			NULL);
	string msg = string_init(msg_buf);
	LocalFree(msg_buf);
	return msg;
}

void Win32Fatal(const char *function) {
	__fatal("%s: %s", function, string_cstr(GetLastErrorString()));
}

int RunCmd(char *command, string *output) {
	SECURITY_ATTRIBUTES security_attributes;
	security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	security_attributes.bInheritHandle = TRUE;

	// Must be inheritable so subprocesses can dup to children.
	HANDLE nul =
			CreateFileA("NUL", GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
					&security_attributes, OPEN_EXISTING, 0, NULL);
	if (nul == INVALID_HANDLE_VALUE) {
		__fatal("couldn't open nul");
	}
	HANDLE stdout_read, stdout_write;
	if (!CreatePipe(&stdout_read, &stdout_write, &security_attributes, 0)) {
		Win32Fatal("CreatePipe");
	}
	if (!SetHandleInformation(stdout_read, HANDLE_FLAG_INHERIT, 0)) {
		Win32Fatal("SetHandleInformation");
	}
	PROCESS_INFORMATION process_info;
	STARTUPINFOA startup_info;
	startup_info.cb = sizeof(STARTUPINFOA);
	startup_info.hStdInput = nul;
	startup_info.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	startup_info.hStdOutput = stdout_write;
	startup_info.dwFlags |= STARTF_USESTDHANDLES;

	if (!CreateProcessA(NULL, command, NULL, NULL,
				/* inherit handles */ TRUE, 0,
				NULL, NULL,
				&startup_info, &process_info)) {
		Win32Fatal("CreateProcess");
	}

	if (!CloseHandle(nul) ||
			!CloseHandle(stdout_write)) {
		Win32Fatal("CloseHandle");
	}

	// Read all output of the subprocess.
	DWORD read_len = 1;
	while (read_len) {
		char buf[64 << 10];
		read_len = 0;
		if (!ReadFile(stdout_read, buf, sizeof(buf), &read_len, NULL) && GetLastError() != ERROR_BROKEN_PIPE) {
			Win32Fatal("ReadFile");
		}
		string_append(output, buf, read_len);
	}

	// Wait for it to exit and grab its exit code.
	if (WaitForSingleObject(process_info.hProcess, INFINITE) == WAIT_FAILED)
		Win32Fatal("WaitForSingleObject");
	DWORD exit_code = 0;
	if (!GetExitCodeProcess(process_info.hProcess, &exit_code))
		Win32Fatal("GetExitCodeProcess");

	if (!CloseHandle(stdout_read) ||
			!CloseHandle(process_info.hProcess) ||
			!CloseHandle(process_info.hThread)) {
		Win32Fatal("CloseHandle");
	}

	return exit_code;
}

int main() {
	char *cc = getenv("CC");

	string output = string_new();
	string command = string_init(cc ? cc : "cl.exe");
	string_cappend(&command, " ");
	string_cappend(&command, GetCommandLineA());
	int exit_code = RunCmd(string_cstr(command), &output);

	_setmode(_fileno(stdout), _O_BINARY);
	// Avoid printf and C strings, since the actual output might contain null bytes like UTF-16 does (yuck).
	fwrite(string_cstr(output), 1, string_size(output), stdout);

	return exit_code;
}
