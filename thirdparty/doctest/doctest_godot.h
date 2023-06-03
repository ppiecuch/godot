#pragma once

#include "core/os/dir_access.h"
#include "core/os/os.h"

String static _doctest_get_folder() {
	return vformat("__doctest__/%04d_%02d_%02d_%02d/",
					OS::get_singleton()->get_date().year,
					OS::get_singleton()->get_date().month,
					OS::get_singleton()->get_date().day,
					OS::get_singleton()->get_time().hour);
}

void static _doctest_prepare_folder() {
	DirAccess::create(DirAccess::ACCESS_FILESYSTEM)->make_dir_recursive(_doctest_get_folder());
}
