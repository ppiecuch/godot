#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

typedef struct {
	char* string;
	int size;
} string;

string string_new() { string s; s->string = malloc(512); s->size = 512; return s; }
string string_init(const char *cstr) { string s; s->string = strdup(cstr); s->size = strlen(cstr); return s; }
void string_append(string *s, const char *data, size_t data_len);
const char *string_cstr(const string *s) { return s->string; }

void __fatal(const char* msg, ...) {
	va_list ap;
	fprintf(stderr, "clw: fatal: ");
	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);
	fprintf(stderr, "\n");
# ifdef _WIN32
	// On Windows, some tools may inject extra threads.
	// exit() may block on locks held by those threads, so forcibly exit.
	fflush(stderr);
	fflush(stdout);
	ExitProcess(1);
# else
	exit(1);
# endif
}

string GetLastErrorString() {
	DWORD err = GetLastError();

	char* msg_buf = NULL;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&msg_buf,
		0,
		NULL);
	string msg = string_init(msg_buf);
	LocalFree(msg_buf);
	return msg;
}

void Win32Fatal(const char* function, const char* hint) {
	if (hint) {
		__fatal("%s: %s (%s)", function, string_cstr(GetLastErrorString()), hint);
	} else {
		__fatal("%s: %s", function, string_cstr(GetLastErrorString()));
	}
}

int RunCmd(const char *command, string *output) {
	SECURITY_ATTRIBUTES security_attributes = {};
	security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	security_attributes.bInheritHandle = TRUE;

	// Must be inheritable so subprocesses can dup to children.
	HANDLE nul =
		CreateFileA("NUL", GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
					&security_attributes, OPEN_EXISTING, 0, NULL);
	if (nul == INVALID_HANDLE_VALUE)
		__fatal("couldn't open nul");

	HANDLE stdout_read, stdout_write;
	if (!CreatePipe(&stdout_read, &stdout_write, &security_attributes, 0))
		Win32Fatal("CreatePipe");

	if (!SetHandleInformation(stdout_read, HANDLE_FLAG_INHERIT, 0))
		Win32Fatal("SetHandleInformation");

	PROCESS_INFORMATION process_info = {};
	STARTUPINFOA startup_info = {};
	startup_info.cb = sizeof(STARTUPINFOA);
	startup_info.hStdInput = nul;
	startup_info.hStdError = ::GetStdHandle(STD_ERROR_HANDLE);
	startup_info.hStdOutput = stdout_write;
	startup_info.dwFlags |= STARTF_USESTDHANDLES;

	if (!CreateProcessA(NULL, (char*)command.c_str(), NULL, NULL,
			/* inherit handles */ TRUE, 0,
			env_block_, NULL,
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
		if (!::ReadFile(stdout_read, buf, sizeof(buf), &read_len, NULL) &&
			GetLastError() != ERROR_BROKEN_PIPE) {
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
	char* command = GetCommandLineA();

	string output;
	int exit_code = RunCmd(command, &output);

	_setmode(_fileno(stdout), _O_BINARY);
	// Avoid printf and C strings, since the actual output might contain null bytes like UTF-16 does (yuck).
	fwrite(&output[0], 1, output.size(), stdout);

	return exit_code;
}
