"""Python 3 compatibility helpers for the Godot build system."""

import io


def isbasestring(s):
    return isinstance(s, (str, bytes))


def open_utf8(filename, mode):
    return open(filename, mode, encoding="utf-8")


def byte_to_str(x):
    return str(x)


def StringIO():
    return io.StringIO()


def encode_utf8(x):
    return x.encode("utf-8") if isinstance(x, str) else x


def decode_utf8(x):
    return x.decode("utf-8") if isinstance(x, bytes) else x


def iteritems(d):
    return iter(d.items())


def itervalues(d):
    return iter(d.values())


def escape_string(s):
    if isinstance(s, str):
        s = s.encode("utf-8")
    result = ""
    for c in s:
        if not (32 <= c < 127) or c in (ord("\\"), ord('"')):
            rev_result = []
            val = c
            while val >= 256:
                val, low = (val // 256, val % 256)
                rev_result.append("\\%03o" % low)
            rev_result.append("\\%03o" % val)
            result += "".join(reversed(rev_result))
        else:
            result += chr(c)
    return result


def qualname(obj):
    return obj.__qualname__
