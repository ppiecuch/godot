"""
SCons-compatible C++ resource compiler.

Port of CMakeRC (https://github.com/elasticdog/transcrypt) for embedding
arbitrary files into C++ binaries via the cmrc::embedded_filesystem API.

Usage from SCons:
    rc = CMRCLibrary("mylib")
    rc.add_resources(["assets/icon.png", "assets/data.json"])
    generated = rc.generate("build/cmrc")
    # Add generated files to your build
"""

import os
import re
import binascii
import hashlib

CMRC_HEADER = """\
#ifndef CMRC_H_INCLUDED
#define CMRC_H_INCLUDED

#include <cassert>
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <mutex>
#include <string>
#include <system_error>
#include <type_traits>

#if !(defined(__EXCEPTIONS) || defined(__cpp_exceptions) || defined(_CPPUNWIND) || defined(CMRC_NO_EXCEPTIONS))
#define CMRC_NO_EXCEPTIONS 1
#endif

namespace cmrc { namespace detail { struct dummy; } }

#define CMRC_DECLARE(libid) \\
    namespace cmrc { namespace detail { \\
    struct dummy; \\
    static_assert(std::is_same<dummy, ::cmrc::detail::dummy>::value, "CMRC_DECLARE() must only appear at the global namespace"); \\
    } } \\
    namespace cmrc { namespace libid { \\
    cmrc::embedded_filesystem get_filesystem(); \\
    } } static_assert(true, "")

namespace cmrc {

class file {
    const char* _begin = nullptr;
    const char* _end = nullptr;

public:
    using iterator = const char*;
    using const_iterator = iterator;
    iterator begin() const noexcept { return _begin; }
    iterator cbegin() const noexcept { return _begin; }
    iterator end() const noexcept { return _end; }
    iterator cend() const noexcept { return _end; }
    std::size_t size() const { return static_cast<std::size_t>(std::distance(begin(), end())); }

    file() = default;
    file(iterator beg, iterator end) noexcept : _begin(beg), _end(end) {}
};

class directory_entry;

namespace detail {

class directory;
class file_data;

class file_or_directory {
    union _data_t {
        class file_data* file_data;
        class directory* directory;
    } _data;
    bool _is_file = true;

public:
    explicit file_or_directory(file_data& f) {
        _data.file_data = &f;
    }
    explicit file_or_directory(directory& d) {
        _data.directory = &d;
        _is_file = false;
    }
    bool is_file() const noexcept {
        return _is_file;
    }
    bool is_directory() const noexcept {
        return !is_file();
    }
    const directory& as_directory() const noexcept {
        assert(!is_file());
        return *_data.directory;
    }
    const file_data& as_file() const noexcept {
        assert(is_file());
        return *_data.file_data;
    }
};

class file_data {
public:
    const char* begin_ptr;
    const char* end_ptr;
    file_data(const file_data&) = delete;
    file_data(const char* b, const char* e) : begin_ptr(b), end_ptr(e) {}
};

inline std::pair<std::string, std::string> split_path(const std::string& path) {
    auto first_sep = path.find("/");
    if (first_sep == path.npos) {
        return std::make_pair(path, "");
    } else {
        return std::make_pair(path.substr(0, first_sep), path.substr(first_sep + 1));
    }
}

struct created_subdirectory {
    class directory& directory;
    class file_or_directory& index_entry;
};

class directory {
    std::list<file_data> _files;
    std::list<directory> _dirs;
    std::map<std::string, file_or_directory> _index;

    using base_iterator = std::map<std::string, file_or_directory>::const_iterator;

public:

    directory() = default;
    directory(const directory&) = delete;

    created_subdirectory add_subdir(std::string name) & {
        _dirs.emplace_back();
        auto& back = _dirs.back();
        auto& fod = _index.emplace(name, file_or_directory{back}).first->second;
        return created_subdirectory{back, fod};
    }

    file_or_directory* add_file(std::string name, const char* begin, const char* end) & {
        assert(_index.find(name) == _index.end());
        _files.emplace_back(begin, end);
        return &_index.emplace(name, file_or_directory{_files.back()}).first->second;
    }

    const file_or_directory* get(const std::string& path) const {
        auto pair = split_path(path);
        auto child = _index.find(pair.first);
        if (child == _index.end()) {
            return nullptr;
        }
        auto& entry  = child->second;
        if (pair.second.empty()) {
            return &entry;
        }
        if (entry.is_file()) {
            return nullptr;
        }
        return entry.as_directory().get(pair.second);
    }

    class iterator {
        base_iterator _base_iter;
        base_iterator _end_iter;
    public:
        using value_type = directory_entry;
        using difference_type = std::ptrdiff_t;
        using pointer = const value_type*;
        using reference = const value_type&;
        using iterator_category = std::input_iterator_tag;

        iterator() = default;
        explicit iterator(base_iterator iter, base_iterator end) : _base_iter(iter), _end_iter(end) {}

        iterator begin() const noexcept {
            return *this;
        }

        iterator end() const noexcept {
            return iterator(_end_iter, _end_iter);
        }

        inline value_type operator*() const noexcept;

        bool operator==(const iterator& rhs) const noexcept {
            return _base_iter == rhs._base_iter;
        }

        bool operator!=(const iterator& rhs) const noexcept {
            return !(*this == rhs);
        }

        iterator operator++() noexcept {
            auto cp = *this;
            ++_base_iter;
            return cp;
        }

        iterator& operator++(int) noexcept {
            ++_base_iter;
            return *this;
        }
    };

    using const_iterator = iterator;

    iterator begin() const noexcept {
        return iterator(_index.begin(), _index.end());
    }

    iterator end() const noexcept {
        return iterator();
    }
};

inline std::string normalize_path(std::string path) {
    while (path.find("/") == 0) {
        path.erase(path.begin());
    }
    while (!path.empty() && (path.rfind("/") == path.size() - 1)) {
        path.pop_back();
    }
    auto off = path.npos;
    while ((off = path.find("//")) != path.npos) {
        path.erase(path.begin() + static_cast<std::string::difference_type>(off));
    }
    return path;
}

using index_type = std::map<std::string, const cmrc::detail::file_or_directory*>;

} // detail

class directory_entry {
    std::string _fname;
    const detail::file_or_directory* _item;

public:
    directory_entry() = delete;
    explicit directory_entry(std::string filename, const detail::file_or_directory& item)
        : _fname(filename)
        , _item(&item)
    {}

    const std::string& filename() const & {
        return _fname;
    }
    std::string filename() const && {
        return std::move(_fname);
    }

    bool is_file() const {
        return _item->is_file();
    }

    bool is_directory() const {
        return _item->is_directory();
    }
};

directory_entry detail::directory::iterator::operator*() const noexcept {
    assert(begin() != end());
    return directory_entry(_base_iter->first, _base_iter->second);
}

using directory_iterator = detail::directory::iterator;

class embedded_filesystem {
    const cmrc::detail::index_type* _index;
    const detail::file_or_directory* _get(std::string path) const {
        path = detail::normalize_path(path);
        auto found = _index->find(path);
        if (found == _index->end()) {
            return nullptr;
        } else {
            return found->second;
        }
    }

public:
    explicit embedded_filesystem(const detail::index_type& index)
        : _index(&index)
    {}

    file open(const std::string& path) const {
        auto entry_ptr = _get(path);
        if (!entry_ptr || !entry_ptr->is_file()) {
#ifdef CMRC_NO_EXCEPTIONS
            fprintf(stderr, "Error no such file or directory: %s\\n", path.c_str());
            abort();
#else
            throw std::system_error(make_error_code(std::errc::no_such_file_or_directory), path);
#endif
        }
        auto& dat = entry_ptr->as_file();
        return file{dat.begin_ptr, dat.end_ptr};
    }

    bool is_file(const std::string& path) const noexcept {
        auto entry_ptr = _get(path);
        return entry_ptr && entry_ptr->is_file();
    }

    bool is_directory(const std::string& path) const noexcept {
        auto entry_ptr = _get(path);
        return entry_ptr && entry_ptr->is_directory();
    }

    bool exists(const std::string& path) const noexcept {
        return !!_get(path);
    }

    directory_iterator iterate_directory(const std::string& path) const {
        auto entry_ptr = _get(path);
        if (!entry_ptr) {
#ifdef CMRC_NO_EXCEPTIONS
            fprintf(stderr, "Error no such file or directory: %s\\n", path.c_str());
            abort();
#else
            throw std::system_error(make_error_code(std::errc::no_such_file_or_directory), path);
#endif
        }
        if (!entry_ptr->is_directory()) {
#ifdef CMRC_NO_EXCEPTIONS
            fprintf(stderr, "Error not a directory: %s\\n", path.c_str());
            abort();
#else
            throw std::system_error(make_error_code(std::errc::not_a_directory), path);
#endif
        }
        return entry_ptr->as_directory().begin();
    }
};

}

#endif // CMRC_H_INCLUDED
"""

_ALPHA_UNDER = re.compile(r"[A-Za-z_]")
_ALPHA_NUM_UNDER = re.compile(r"[A-Za-z0-9_]")


def _make_c_identifier(string):
    """Convert a string to a valid C identifier, matching CMake's MAKE_C_IDENTIFIER behavior."""
    result = []
    for i, c in enumerate(string):
        pattern = _ALPHA_UNDER if i == 0 else _ALPHA_NUM_UNDER
        if pattern.match(c):
            result.append(c)
        else:
            if i == 0:
                result.append("_")
            result.append("_")
    return "".join(result)


def _encode_fpath(fpath):
    """Generate a unique C identifier from a resource path."""
    digest = hashlib.md5(fpath.encode("utf-8")).hexdigest()[:8]
    return "f_%s_%s" % (digest, _make_c_identifier(fpath))


def _hex_literal(content):
    """Convert binary content to a comma-separated hex literal string."""
    hex_str = binascii.hexlify(content)
    return ",".join("0x" + hex_str[i : i + 2].decode("ascii") for i in range(0, len(hex_str), 2))


class CMRCLibrary:
    """
    SCons-compatible C++ resource compiler.

    Embeds arbitrary files into a C++ binary, accessible at runtime
    via the cmrc::embedded_filesystem API.
    """

    def __init__(self, namespace):
        """
        Create a new resource library.

        Args:
            namespace: C++ namespace for the library (must be a valid identifier).
        """
        self.namespace = namespace
        self._files = []  # list of (resource_path, symbol, disk_path)
        self._dirs = set()  # set of directory resource paths

    def add_resources(self, filepaths, prefix="", whence=""):
        """
        Add files to the resource library.

        Args:
            filepaths: A file path or list of file paths to embed.
            prefix:    Prepend this directory prefix to all resource paths.
            whence:    Rewrite file paths relative to this directory.
        """
        if isinstance(filepaths, str):
            filepaths = [filepaths]

        for filepath in filepaths:
            if not os.path.isfile(filepath):
                raise FileNotFoundError("Resource file not found: %s" % filepath)

            # Compute the resource path (how the file is accessed at runtime)
            relpath = os.path.relpath(filepath, whence) if whence else filepath
            relpath = relpath.replace("\\", "/")
            if prefix:
                relpath = prefix.strip("/") + "/" + relpath

            sym = _encode_fpath(relpath)
            self._files.append((relpath, sym, filepath))

            # Register all parent directories
            parts = relpath.split("/")
            for i in range(1, len(parts)):
                self._dirs.add("/".join(parts[:i]))

    def generate(self, output_dir="build"):
        """
        Generate cmrc.h and the compiled resource .cpp file.

        Args:
            output_dir: Directory to write generated files into.

        Returns:
            List of generated file paths [header, source].
        """
        os.makedirs(output_dir, exist_ok=True)

        header_path = os.path.join(output_dir, "cmrc.h")
        source_path = os.path.join(output_dir, "cmrc_%s.cpp" % self.namespace)

        with open(header_path, "w") as f:
            f.write(CMRC_HEADER)

        with open(source_path, "w") as f:
            f.write(self._generate_source())

        return [header_path, source_path]

    def _generate_source(self):
        """Build the complete .cpp source as a string."""
        lines = []
        lines.append('#include "cmrc.h"')
        lines.append("")

        # Emit file data arrays and extern pointers
        for relpath, sym, filepath in self._files:
            with open(filepath, "rb") as f:
                content = f.read()
            n_bytes = len(content)

            lines.append("// %s" % relpath)
            if n_bytes > 0:
                lines.append("namespace { const char %s_array[] = { %s }; }" % (sym, _hex_literal(content)))
            else:
                lines.append("namespace { const char %s_array[] = { 0 }; }" % sym)

            lines.append("namespace cmrc { namespace %s { namespace res_chars {" % self.namespace)
            lines.append("extern const char* const %s_begin = %s_array;" % (sym, sym))
            lines.append("extern const char* const %s_end = %s_array + %d;" % (sym, sym, n_bytes))
            lines.append("}}}")
            lines.append("")

        # Filesystem index setup
        lines.append("namespace cmrc { namespace %s {" % self.namespace)
        lines.append("")
        lines.append("namespace {")
        lines.append("")
        lines.append("const cmrc::detail::index_type&")
        lines.append("get_root_index() {")
        lines.append("    static cmrc::detail::directory root_directory_;")
        lines.append("    static cmrc::detail::file_or_directory root_directory_fod{root_directory_};")
        lines.append("    static cmrc::detail::index_type root_index;")
        lines.append('    root_index.emplace("", &root_directory_fod);')
        lines.append("    struct dir_inl { class cmrc::detail::directory& directory; };")
        lines.append("    dir_inl root_directory_dir{root_directory_};")
        lines.append("    (void)root_directory_dir;")

        # Create subdirectories (sorted so parents always precede children)
        for dirpath in sorted(self._dirs):
            dir_sym = _encode_fpath(dirpath)
            parts = dirpath.split("/")
            leaf = parts[-1]
            if len(parts) == 1:
                parent_sym = "root_directory"
            else:
                parent_sym = _encode_fpath("/".join(parts[:-1]))
            lines.append('    static auto %s_dir = %s_dir.directory.add_subdir("%s");' % (dir_sym, parent_sym, leaf))
            lines.append('    root_index.emplace("%s", &%s_dir.index_entry);' % (dirpath, dir_sym))

        # Register files in their parent directories
        for relpath, sym, _ in self._files:
            parts = relpath.split("/")
            leaf = parts[-1]
            if len(parts) == 1:
                parent_sym = "root_directory"
            else:
                parent_sym = _encode_fpath("/".join(parts[:-1]))
            lines.append("    root_index.emplace(")
            lines.append('        "%s",' % relpath)
            lines.append("        %s_dir.directory.add_file(" % parent_sym)
            lines.append('            "%s",' % leaf)
            lines.append("            res_chars::%s_begin," % sym)
            lines.append("            res_chars::%s_end" % sym)
            lines.append("        )")
            lines.append("    );")

        lines.append("    return root_index;")
        lines.append("}")
        lines.append("")
        lines.append("} // anonymous namespace")
        lines.append("")
        lines.append("cmrc::embedded_filesystem get_filesystem() {")
        lines.append("    static auto& index = get_root_index();")
        lines.append("    return cmrc::embedded_filesystem{index};")
        lines.append("}")
        lines.append("")
        lines.append("} // %s" % self.namespace)
        lines.append("} // cmrc")
        lines.append("")

        return "\n".join(lines)


"""
# CMakeRC - A Standalone CMake-Based C++ Resource Compiler
# --------------------------------------------------------

CMakeRC is a resource compiler provided in a single CMake script that can
easily be included in another project.

## What is a "Resource Compiler"?

For the purpose of this project, a _resource compiler_ is a tool that will
compile arbitrary data into a program. The program can then read this data from
without needing to store that data on disk external to the program.

Examples use cases:

- Storing a web page tree for serving over HTTP to clients. Compiling the web
  page into the executable means that the program is all that is required to
  run the HTTP server, without keeping the site files on disk separately.
- Storing embedded scripts and/or shaders that support the program, rather than
  writing them in the code as string literals.
- Storing images and graphics for GUIs.

These things are all about aiding in the ease of portability and distribution
of the program, as it is no longer required to ship a plethora of support files
with a binary to your users.

## What is Special About CMakeRC?

CMakeRC is implemented as a single CMake module, `CMakeRC.cmake`. No additional
libraries or headers are required.

This project was initially written as a "literate programming" experiment.
The process for the pre-2.0 version can be read about here:
https://vector-of-bool.github.io/2017/01/21/cmrc.html.

2.0.0+ is slightly different from what was written in the post, but a lot of it
still applies.

## Installing

Installing CMakeRC is designed to be as simple as possible. The only thing
required is the `CMakeRC.cmake` script. You can copy it into your project
directory (recommended) or install it as a package and get all the features you
need.

## Usage

1. Once installed, simply import the `CMakeRC.cmake` script. If you placed the
   module in your project directory (recommended), simply use
   `include(CMakeRC)` to import the module. If you installed it as a package,
   use `find_package(CMakeRC)`.

2. Once included, create a new resource library using
   `cmrc_add_resource_library`, like this:

   ```cmake
   cmrc_add_resource_library(foo-resources ...)
   ```

   Where `...` is simply a list of files that you wish to compile into the
   resource library.

  You can use the `ALIAS` argument to immediately generate an alias target for
  the resource library (recommended):

  ```cmake
  cmrc_add_resource_library(foo-resources ALIAS foo::rc ...)
  ```

  **Note:** If the name of the library target is not a valid C++ `namespace`
  identifier, you will need to provide the `NAMESPACE` argument. Otherwise, the
  name of the library will be used as the resource library's namespace.

  ```cmake
  cmrc_add_resource_library(foo-resources ALIAS foo::rc NAMESPACE foo  ...)
  ```

3. To use the resource library, link the resource library target into a binary
   using `target_link_libraries()`:

   ```cmake
   add_executable(my-program main.cpp)
   target_link_libraries(my-program PRIVATE foo::rc)
   ```

4. Inside of the source files, any time you wish to use the library, include
   the `cmrc/cmrc.h` header, which will automatically become available to any
   target that links to a generated resource library target, as `my-program`
   does above:

   ```c++
   #include <cmrc/cmrc.h>

   int main() {
       // ...
   }
   ```

5. At global scope within the `.cpp` file, place the
   `CMRC_DECLARE(<my-lib-ns>)` macro using the namespace that was designated
   with `cmrc_add_resource_library` (or the library name if no namespace was
   specified):

   ```c++
   #include <cmrc/cmrc.h>

   CMRC_DECLARE(foo);

   int main() {
       // ...
   }
   ```

6. Obtain a handle to the embedded resource filesystem by calling the
   `get_filesystem()` function in the generated namespace. It will be
   generated at `cmrc::<my-lib-ns>::get_filesystem()`.

   ```c++
   int main() {
       auto fs = cmrc::foo::get_filesystem();
   }
   ```

   (This function was declared by the `CMRC_DECLARE()` macro from the previous
   step.)

   You're now ready to work with the files in your resource library!
   See the section on `cmrc::embedded_filesystem`.

## The `cmrc::embedded_filesystem` API

All resource libraries have their own `cmrc::embedded_filesystem` that can be
accessed with the `get_filesystem()` function declared by `CMRC_DECLARE()`.

This class is trivially copyable and destructible, and acts as a handle to the
statically allocated resource library data.

### Methods on `cmrc::embedded_filesystem`

- `open(const std::string& path) -> cmrc::file` - Opens and returns a
  non-directory `file` object at `path`, or throws `std::system_error()` on
  error.
- `is_file(const std::string& path) -> bool` - Returns `true` if the given
  `path` names a regular file, `false` otherwise.
- `is_directory(const std::string& path) -> bool` - Returns `true` if the given
  `path` names a directory. `false` otherwise.
- `exists(const std::string& path) -> bool` returns `true` if the given path
  names an existing file or directory, `false` otherwise.
- `iterate_directory(const std::string& path) -> cmrc::directory_iterator`
  returns a directory iterator for iterating the contents of a directory.
  Throws if the given `path` does not identify a directory.

## Members of `cmrc::file`

- `typename iterator` and `typename const_iterator` - Just `const char*`.
- `begin()/cbegin() -> iterator` - Return an iterator to the beginning of the
  resource.
- `end()/cend() -> iterator` - Return an iterator past the end of the resource.
- `file()` - Default constructor, refers to no resource.

## Members of `cmrc::directory_iterator`

- `typename value_type` - `cmrc::directory_entry`
- `iterator_category` - `std::input_iterator_tag`
- `directory_iterator()` - Default construct.
- `begin() -> directory_iterator` - Returns `*this`.
- `end() -> directory_iterator` - Returns a past-the-end iterator corresponding
  to this iterator.
- `operator*() -> value_type` - Returns the `directory_entry` for which the
  iterator corresponds.
- `operator==`, `operator!=`, and `operator++` - Implement iterator semantics.

## Members of `cmrc::directory_entry`

- `filename() -> std::string` - The filename of the entry.
- `is_file() -> bool` - `true` if the entry is a file.
- `is_directory() -> bool` - `true` if the entry is a directory.

## Additional Options

After calling `cmrc_add_resource_library`, you can add additional resources to
the library using `cmrc_add_resources` with the name of the library and the
paths to any additional resources that you wish to compile in. This way you can
lazily add resources to the library as your configure script runs.

Both `cmrc_add_resource_library` and `cmrc_add_resources` take two additional
keyword parameters:

- `WHENCE` tells CMakeRC how to rewrite the filepaths to the resource files.
  The default value for `WHENCE` is the `CMAKE_CURRENT_SOURCE_DIR`, which is
  the source directory where `cmrc_add_resources` or
  `cmrc_add_resource_library` is called. For example, if you say
  `cmrc_add_resources(foo images/flower.jpg)`, the resource will be accessible
  via `cmrc::open("images/flower.jpg")`, but if you say
  `cmrc_add_resources(foo WHENCE images images/flower.jpg)`, then the resource
  will be accessible only using `cmrc::open("flower.jpg")`, because the
  `images` directory is used as the root where the resource will be compiled
  from.

  Because of the file transformation limitations, `WHENCE` is _required_ when
  adding resources which exist outside of the source directory, since CMakeRC
  will not be able to automatically rewrite the file paths.

- `PREFIX` tells CMakeRC to prepend a directory-style path to the resource
  filepath in the resulting binary. For example,
  `cmrc_add_resources(foo PREFIX resources images/flower.jpg)` will make the
  resource accessible using `cmrc::open("resources/images/flower.jpg")`.
  This is useful to prevent resource libraries from having conflicting
  filenames. The default `PREFIX` is to have no prefix.

The two options can be used together to rewrite the paths to your heart's
content:

```cmake
cmrc_add_resource_library(
    flower-images
    NAMESPACE flower
    WHENCE images
    PREFIX flowers
    images/rose.jpg
    images/tulip.jpg
    images/daisy.jpg
    images/sunflower.jpg
    )
```

```c++
int foo() {
    auto fs = cmrc::flower::get_filesystem();
    auto rose = fs.open("flowers/rose.jpg");
}
```
"""
