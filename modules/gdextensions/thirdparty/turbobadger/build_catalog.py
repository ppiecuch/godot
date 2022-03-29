#! env python

import os
import sys
import random

def get_id(filename):
    return os.path.splitext(filename)[0].replace("-", "_").replace("@", "_").rstrip(".tb")

walk_dir = sys.argv[1]

# pass 1
for root, subdirs, files in os.walk(walk_dir):
    files = [ fi for fi in files if fi.endswith(".png") or fi.endswith(".txt") ]

    for filename in files:
        id = get_id(filename)
        print("extern unsigned char *%s;" % id)
        print("extern unsigned int %s_size;" % id)

# pass 2
count = 0
rnd = random.randint(10000000, 99999999)
print("const _finfo_t __fa_%s[] = {" % rnd)
for root, subdirs, files in os.walk(walk_dir):
    files = [ fi for fi in files if fi.endswith(".png") or fi.endswith(".txt") ]

    for filename in files:
        file_path = os.path.join(root, filename)
        id = get_id(filename)
        print("  { \"%s\", %s, %s_size }," % (file_path, id, id))
        count += 1
print("};")

print("register_mem_files(__fa_%s, %d);" % (rnd, count))
