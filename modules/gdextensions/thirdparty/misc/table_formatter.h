#ifndef TABLE_FORMATTER_H
#define TABLE_FORMATTER_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>

namespace tblfmt {

#if defined(__unix__) || defined(__unix) || defined(__linux__)
# define OS_LINUX
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
# define OS_WIN
#elif defined(__APPLE__) || defined(__MACH__)
# define OS_MAC
#else
#error Unknown Platform
#endif

#if defined(OS_LINUX) || defined(OS_MAC)
# include <unistd.h>
#elif defined(OS_WIN)
# if defined(_WIN32_WINNT) && (_WIN32_WINNT < 0x0600)
# error "Please include rang.hpp before any windows system headers or set _WIN32_WINNT at least to _WIN32_WINNT_VISTA"
# elif !defined(_WIN32_WINNT)
# define _WIN32_WINNT _WIN32_WINNT_VISTA
# endif

#include <windows.h>
#include <io.h>
#include <memory>

// Only defined in windows 10 onwards, redefining in lower windows since it
// doesn't gets used in lower versions
// https://docs.microsoft.com/en-us/windows/console/getconsolemode

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#endif

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace rang {
	// For better compability with most of terminals do not use any style settings
	// except of reset, bold and reversed.
	// Note that on Windows terminals bold style is same as fgB color.
	enum class style {
		reset     = 0,
		bold      = 1,
		dim       = 2,
		italic    = 3,
		underline = 4,
		blink     = 5,
		rblink    = 6,
		reversed  = 7,
		conceal   = 8,
		crossed   = 9,
	};

	enum class fg {
		black   = 30,
		red     = 31,
		green   = 32,
		yellow  = 33,
		blue    = 34,
		magenta = 35,
		cyan    = 36,
		gray    = 37,
		reset   = 39,
	};

	enum class bg {
		black   = 40,
		red     = 41,
		green   = 42,
		yellow  = 43,
		blue    = 44,
		magenta = 45,
		cyan    = 46,
		gray    = 47,
		reset   = 49,
	};

	enum class fgB {
		black   = 90,
		red     = 91,
		green   = 92,
		yellow  = 93,
		blue    = 94,
		magenta = 95,
		cyan    = 96,
		gray    = 97,
	};

	enum class bgB {
		black   = 100,
		red     = 101,
		green   = 102,
		yellow  = 103,
		blue    = 104,
		magenta = 105,
		cyan    = 106,
		gray    = 107,
	};

	enum class control {
		// Behaviour of rang function calls
		Off   = 0,  // toggle off rang style/color calls
		Auto  = 1,  // (Default) autodect terminal and colorize if needed
		Force = 2  // force ansi color output to non terminal streams
	};

	// Use rang::setControlMode to set rang control mode

	enum class winTerm {
		// Windows Terminal Mode
		Auto   = 0,  // (Default) automatically detects wheter Ansi or Native API
		Ansi   = 1,  // Force use Ansi API
		Native = 2  // Force use Native API
	};

	// Use rang::setWinTermMode to explicitly set terminal API for Windows
	// Calling rang::setWinTermMode have no effect on other OS

	namespace rang_implementation {
		inline std::atomic<control>& controlMode() noexcept {
			static std::atomic<control> value(control::Auto);
			return value;
		}
		inline std::atomic<winTerm>& winTermMode() noexcept {
			static std::atomic<winTerm> termMode(winTerm::Auto);
			return termMode;
		}
		inline bool supportsColor() noexcept {
#if defined(OS_LINUX) || defined(OS_MAC)
			static const bool result = [] {
				const char* Terms[]
								= {"ansi", "color", "console", "cygwin", "gnome",
									"konsole", "kterm", "linux", "msys", "putty",
									"rxvt", "screen", "vt100", "xterm"};
				const char* env_p = std::getenv("TERM");
				if (env_p == nullptr) {
					return false;
				}
				return std::any_of(std::begin(Terms), std::end(Terms),
								[&](const char* term) {
									return std::strstr(env_p, term) != nullptr;
								});
			}();

#elif defined(OS_WIN)
			// All windows versions support colors through native console methods
			static constexpr bool result = true;
#endif
			return result;
		}

#ifdef OS_WIN
		inline bool isMsysPty(int fd) noexcept {
			// Dynamic load for binary compability with old Windows
			const auto ptrGetFileInformationByHandleEx
			= reinterpret_cast<decltype(&GetFileInformationByHandleEx)>(
				GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")),
							"GetFileInformationByHandleEx"));
			if (!ptrGetFileInformationByHandleEx) {
				return false;
			}

			HANDLE h = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
			if (h == INVALID_HANDLE_VALUE) {
				return false;
			}

			// Check that it's a pipe:
			if (GetFileType(h) != FILE_TYPE_PIPE) {
				return false;
			}

			// POD type is binary compatible with FILE_NAME_INFO from WinBase.h
			// It have the same alignment and used to avoid UB in caller code
			struct MY_FILE_NAME_INFO {
				DWORD FileNameLength;
				WCHAR FileName[MAX_PATH];
			};

			auto pNameInfo = std::unique_ptr<MY_FILE_NAME_INFO>(
			new (std::nothrow) MY_FILE_NAME_INFO());
			if (!pNameInfo) {
				return false;
			}

			// Check pipe name is template of
			// {"cygwin-","msys-"}XXXXXXXXXXXXXXX-ptyX-XX
			if (!ptrGetFileInformationByHandleEx(h, FileNameInfo, pNameInfo.get(),
												sizeof(MY_FILE_NAME_INFO))) {
				return false;
			}
			std::wstring name(pNameInfo->FileName, pNameInfo->FileNameLength / sizeof(WCHAR));
			if ((name.find(L"msys-") == std::wstring::npos
				&& name.find(L"cygwin-") == std::wstring::npos)
				|| name.find(L"-pty") == std::wstring::npos) {
				return false;
			}

			return true;
		}

#endif

		inline bool isTerminal(const std::streambuf* osbuf) noexcept {
			using std::cerr;
			using std::clog;
			using std::cout;
#if defined(OS_LINUX) || defined(OS_MAC)
			if (osbuf == cout.rdbuf()) {
				static const bool cout_term = isatty(fileno(stdout)) != 0;
				return cout_term;
			} else if (osbuf == cerr.rdbuf() || osbuf == clog.rdbuf()) {
				static const bool cerr_term = isatty(fileno(stderr)) != 0;
				return cerr_term;
			}
#elif defined(OS_WIN)
			if (osbuf == cout.rdbuf()) {
				static const bool cout_term
				= (_isatty(_fileno(stdout)) || isMsysPty(_fileno(stdout)));
				return cout_term;
			} else if (osbuf == cerr.rdbuf() || osbuf == clog.rdbuf()) {
				static const bool cerr_term
				= (_isatty(_fileno(stderr)) || isMsysPty(_fileno(stderr)));
				return cerr_term;
			}
#endif
			return false;
		}

		template<typename T>
		using enableStd = typename std::enable_if<
				std::is_same<T, rang::style>::value || std::is_same<T, rang::fg>::value
						|| std::is_same<T, rang::bg>::value || std::is_same<T, rang::fgB>::value
						|| std::is_same<T, rang::bgB>::value,
				std::ostream&>::type;

#ifdef OS_WIN

		struct SGR {  // Select Graphic Rendition parameters for Windows console
			BYTE fgColor;  // foreground color (0-15) lower 3 rgb bits + intense bit
			BYTE bgColor;  // background color (0-15) lower 3 rgb bits + intense bit
			BYTE bold;  // emulated as FOREGROUND_INTENSITY bit
			BYTE underline;  // emulated as BACKGROUND_INTENSITY bit
			BOOLEAN inverse;  // swap foreground/bold & background/underline
			BOOLEAN conceal;  // set foreground/bold to background/underline
		};

		enum class AttrColor : BYTE { // Color attributes for console screen buffer
			black   = 0,
			red     = 4,
			green   = 2,
			yellow  = 6,
			blue    = 1,
			magenta = 5,
			cyan    = 3,
			gray    = 7
		};

		inline HANDLE getConsoleHandle(const std::streambuf *osbuf) noexcept {
			if (osbuf == std::cout.rdbuf()) {
				static const HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
				return hStdout;
			} else if (osbuf == std::cerr.rdbuf() || osbuf == std::clog.rdbuf()) {
				static const HANDLE hStderr = GetStdHandle(STD_ERROR_HANDLE);
				return hStderr;
			}
			return INVALID_HANDLE_VALUE;
		}

		inline bool setWinTermAnsiColors(const std::streambuf *osbuf) noexcept {
			HANDLE h = getConsoleHandle(osbuf);
			if (h == INVALID_HANDLE_VALUE) {
				return false;
			}
			DWORD dwMode = 0;
			if (!GetConsoleMode(h, &dwMode)) {
				return false;
			}
			dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
			if (!SetConsoleMode(h, dwMode)) {
				return false;
			}
			return true;
		}

		inline bool supportsAnsi(const std::streambuf *osbuf) noexcept {
			using std::cerr;
			using std::clog;
			using std::cout;
			if (osbuf == cout.rdbuf()) {
				static const bool cout_ansi
				= (isMsysPty(_fileno(stdout)) || setWinTermAnsiColors(osbuf));
				return cout_ansi;
			} else if (osbuf == cerr.rdbuf() || osbuf == clog.rdbuf()) {
				static const bool cerr_ansi
				= (isMsysPty(_fileno(stderr)) || setWinTermAnsiColors(osbuf));
				return cerr_ansi;
			}
			return false;
		}

		inline const SGR &defaultState() noexcept {
			static const SGR defaultSgr = []() -> SGR {
				CONSOLE_SCREEN_BUFFER_INFO info;
				WORD attrib = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
				if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
											&info)
					|| GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE),
												&info)) {
					attrib = info.wAttributes;
				}
				SGR sgr     = { 0, 0, 0, 0, FALSE, FALSE };
				sgr.fgColor = attrib & 0x0F;
				sgr.bgColor = (attrib & 0xF0) >> 4;
				return sgr;
			}();
			return defaultSgr;
		}

		inline BYTE ansi2attr(BYTE rgb) noexcept {
			static const AttrColor rev[8]
			= { AttrColor::black,  AttrColor::red,  AttrColor::green,
				AttrColor::yellow, AttrColor::blue, AttrColor::magenta,
				AttrColor::cyan,   AttrColor::gray };
			return static_cast<BYTE>(rev[rgb]);
		}

		inline void setWinSGR(rang::bg col, SGR &state) noexcept {
			if (col != rang::bg::reset) {
				state.bgColor = ansi2attr(static_cast<BYTE>(col) - 40);
			} else {
				state.bgColor = defaultState().bgColor;
			}
		}

		inline void setWinSGR(rang::fg col, SGR &state) noexcept {
			if (col != rang::fg::reset) {
				state.fgColor = ansi2attr(static_cast<BYTE>(col) - 30);
			} else {
				state.fgColor = defaultState().fgColor;
			}
		}

		inline void setWinSGR(rang::bgB col, SGR &state) noexcept {
			state.bgColor = (BACKGROUND_INTENSITY >> 4)
			| ansi2attr(static_cast<BYTE>(col) - 100);
		}

		inline void setWinSGR(rang::fgB col, SGR &state) noexcept {
			state.fgColor
			= FOREGROUND_INTENSITY | ansi2attr(static_cast<BYTE>(col) - 90);
		}

		inline void setWinSGR(rang::style style, SGR &state) noexcept {
			switch (style) {
				case rang::style::reset: state = defaultState(); break;
				case rang::style::bold: state.bold = FOREGROUND_INTENSITY; break;
				case rang::style::underline:
				case rang::style::blink: state.underline = BACKGROUND_INTENSITY; break;
				case rang::style::reversed: state.inverse = TRUE; break;
				case rang::style::conceal: state.conceal = TRUE; break;
				default: break;
			}
		}

		inline SGR &current_state() noexcept {
			static SGR state = defaultState();
			return state;
		}

		inline WORD SGR2Attr(const SGR &state) noexcept {
			WORD attrib = 0;
			if (state.conceal) {
				if (state.inverse) {
					attrib = (state.fgColor << 4) | state.fgColor;
					if (state.bold)
						attrib |= FOREGROUND_INTENSITY | BACKGROUND_INTENSITY;
				} else {
					attrib = (state.bgColor << 4) | state.bgColor;
					if (state.underline)
						attrib |= FOREGROUND_INTENSITY | BACKGROUND_INTENSITY;
				}
			} else if (state.inverse) {
				attrib = (state.fgColor << 4) | state.bgColor;
				if (state.bold) attrib |= BACKGROUND_INTENSITY;
				if (state.underline) attrib |= FOREGROUND_INTENSITY;
			} else {
				attrib = state.fgColor | (state.bgColor << 4) | state.bold
				| state.underline;
			}
			return attrib;
		}

		template <typename T> inline void setWinColorAnsi(std::ostream &os, T const value) {
			os << "\033[" << static_cast<int>(value) << "m";
		}

		template <typename T> inline void setWinColorNative(std::ostream &os, T const value) {
			const HANDLE h = getConsoleHandle(os.rdbuf());
			if (h != INVALID_HANDLE_VALUE) {
				setWinSGR(value, current_state());
				// Out all buffered text to console with previous settings:
				os.flush();
				SetConsoleTextAttribute(h, SGR2Attr(current_state()));
			}
		}

		template <typename T> inline enableStd<T> setColor(std::ostream &os, T const value) {
			if (winTermMode() == winTerm::Auto) {
				if (supportsAnsi(os.rdbuf())) {
					setWinColorAnsi(os, value);
				} else {
					setWinColorNative(os, value);
				}
			} else if (winTermMode() == winTerm::Ansi) {
				setWinColorAnsi(os, value);
			} else {
				setWinColorNative(os, value);
			}
			return os;
		}
#else
		template<typename T> inline enableStd<T> setColor(std::ostream& os, T const value) {
			return os << "\033[" << static_cast<int>(value) << "m";
		}

#endif
	}  // namespace rang_implementation

	template<typename T>
	inline rang_implementation::enableStd<T> operator<<(std::ostream& os, const T value) {
		const control option = rang_implementation::controlMode();
		switch (option) {
			case control::Auto:
				return rang_implementation::supportsColor()
							&& rang_implementation::isTerminal(os.rdbuf())
					? rang_implementation::setColor(os, value)
					: os;
			case control::Force:
				return rang_implementation::setColor(os, value);
			default:
				return os;
		}
	}
	inline void setWinTermMode(const rang::winTerm value) noexcept {
		rang_implementation::winTermMode() = value;
	}
	inline void setControlMode(const control value) noexcept {
		rang_implementation::controlMode() = value;
	}
} // namespace rang

#undef OS_LINUX
#undef OS_WIN
#undef OS_MAC

class endl{};

// TablePrinter
//
// Print a pretty table into your output of choice.
//
// Usage:
//   TablePrinter tp(&std::cout);
//   tp.AddColumn("Name", 25);
//   tp.AddColumn("Age", 3);
//   tp.AddColumn("Position", 30);
//
//   tp.PrintHeader();
//   tp << "Dat Chu" << 25 << "Research Assistant";
//   tp << "John Doe" << 26 << "Professional Anonymity";
//   tp << "Jane Doe" << tp.SkipToNextLine();
//   tp << "Tom Doe" << 7 << "Student";
//   tp.PrintFooter();

class TablePrinter {
	template<typename T> void OutputDecimalNumber(T input);

	std::ostream &out_stream_;
	std::vector<std::string> column_headers_;
	std::vector<int> column_widths_;
	std::string separator_;

	int i_; // index of current row
	int j_; // index of current column

	int table_width_;
	bool flush_left_;

	void PrintHorizontalLine(char character = '-') {
		out_stream_ << "+"; // the left bar
		for (int i = 0; i < m_tableWidth - 1; ++i)
			out_stream_ << character;
		out_stream_ << "+"; // the right bar
		out_stream_ << "\n";
	}

public:
	int GetColumnCount() const { return column_headers_.size(); }
	int GetTableWidth() const { return table_width_; }
	void SetSeparator(const std::string &separator) { separator_ = separator; }
	void SetFlushLeft() { flush_left_ = true; }
	void SetFlushRight() { flush_left_ = false; }

	bool AddColumn(const std::string &header_name, int column_width) {
		if (column_width < 4) {
			return false; // "Column width has to be >= 4"
		}

		column_headers_.emplace_back(header_name);
		column_widths_.emplace_back(column_width);
		table_width_ += column_width + separator_.size(); // for the separator

		return true;
	}
	void PrintTitle(const std::string &title) {
		int total_width = 0;
		for (int &it : column_widths_) total_width += it;
		total_width += column_widths_.size() - 1;

		std::string tit = title;
		int title_width = tit.length();
		if (title_width > total_width) tit = tit.substr(0, total_width);

		int pre  = (total_width - tit.length()) / 2;
		int post = (total_width - tit.length() - pre);

		PrintHorizontalLine('=');
		out_stream_ << "|";
		for (int i = 0; i < pre; ++i) out_stream_ << " ";
		out_stream_ << tit;
		for (int i = 0; i < post; ++i) out_stream_ << " ";
		out_stream_ << "|\n";
	}
	void PrintHeader() {
		PrintHorizontalLine('=');
		out_stream_ << "|";
		for (int i = 0; i < GetColumnCount(); ++i) {
			if (flush_left_)
				out_stream_ << std::left;
			else
				out_stream_ << std::right;
			out_stream_ << std::setw(column_widths_.at(i)) << column_headers_.at(i).substr(0, column_widths_.at(i));
			if (i != GetColumnCount() - 1) {
				out_stream_ << separator_;
			}
		}

		out_stream_ << "|\n";
		PrintHorizontalLine('=');
	}
	void PrintFooter() { PrintHorizontalLine(); }

	TablePrinter& operator<<(endl input) {
		while (j_ != 0) {
			*this << "";
		}
		return *this;
	}
	template<typename T> TablePrinter& operator<<(T input) {
		if constexpr(std::is_floating_point<T>::value) {
			OutputDecimalNumber<T>(input);
		} else {
			if (j_ == 0)
				out_stream_ << "|";

			if (flush_left_)
				out_stream_ << std::left;
			else
				out_stream_ << std::right;

			// Leave 3 extra space: One for negative sign, one for zero, one for decimal
			out_stream_ << std::setw(column_widths_.at(j_));
			out_stream_ << input;

			if (j_ == GetColumnCount() - 1) {
				out_stream_ << "|\n";
				i_    = i_ + 1;
				j_ = 0;
			} else {
				out_stream_ << separator_;
				++j_;
			}
		}
		return *this;
	}

	TablePrinter(std::ostream &output = std::cout, const std::string &separator = "|")
	: out_stream_(output)
	, separator_(separator)
	, flush_left_(false) { }
	~TablePrinter();
};

template<typename T> void TablePrinter::OutputDecimalNumber(T input) {
	// If we cannot handle this number, indicate so
	if (input < 10*(column_widths_.at(j_)-1) || input > 10*column_widths_.at(j_)) {
		std::stringstream string_out;
		string_out << std::setiosflags(std::ios::fixed)
				<< std::setprecision(column_widths_.at(j_))
				<< std::setw(column_widths_.at(j_))
				<< input;

		std::string string_rep_of_number = string_out.str();

		string_rep_of_number[column_widths_.at(j_)-1] = '*';
		std::string string_to_print = string_rep_of_number.substr(0, column_widths_.at(j_));
		out_stream_ << string_to_print;

	} else {

		// determine what precision we need
		int precision = column_widths_.at(j_) - 1; // leave room for the decimal point
		if (input < 0)
			--precision; // leave room for the minus sign
		// leave room for digits before the decimal?
		if (input < -1 || input > 1) {
			int num_digits_before_decimal = 1 + static_cast<int>(log10(std::abs(input)));
			precision -= num_digits_before_decimal;
		} else
			precision --; // e.g. 0.12345 or -0.1234

		if (precision < 0)
			precision = 0; // don't go negative with precision

		out_stream_ << std::setiosflags(std::ios::fixed)
					<< std::setprecision(precision)
					<< std::setw(column_widths_.at(j_))
					<< input;
	}

	if (j_ == GetColumnCount()-1) {
		out_stream_ << "|\n";
		i_ = i_ + 1;
		j_ = 0;
	} else {
		out_stream_ << separator_;
		j_ = j_ + 1;
	}
}

} // namspace tblfmt

#endif // TABLE_FORMATTER_H
