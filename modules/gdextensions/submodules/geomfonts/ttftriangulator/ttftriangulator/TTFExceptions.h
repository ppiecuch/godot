#pragma once

#include <exception>
#include <cstdint>
#include <string>

namespace TTFCore {

struct FontException : public std::exception {
	std::string msg;

	FontException();
	FontException(const char* msg);
	FontException(const std::string& msg);
	virtual const char* what() const throw();
};

struct FileFailure : public FontException { 
	FileFailure(const std::string& flnm);
};

struct FileLengthError : public FontException { 
	FileLengthError();
	FileLengthError(const std::string& flnm);
};

struct TableDoesNotExist : public FontException {
	TableDoesNotExist(const std::string& table);
};

struct ChecksumException : public FontException { 
	ChecksumException(const std::string& table);
};

struct VersionException : public FontException { 
	VersionException(const std::string& msg);
};

struct InvalidFontException : public FontException { 
	InvalidFontException(const std::string& msg);
};

struct UnsupportedCap : public FontException { 
	UnsupportedCap(const std::string& msg);
};

struct InvalidContour : public FontException { 
	InvalidContour(const std::string& msg);
};

struct InternalError : public FontException { 
	InternalError(const std::string& msg);
};

} // namespace
