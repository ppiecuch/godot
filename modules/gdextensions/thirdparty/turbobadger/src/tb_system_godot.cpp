// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_system.h"

#ifdef TB_SYSTEM_GODOT

#include "core/os/os.h"

#ifdef TB_RUNTIME_DEBUG_INFO

#include "core/print_string.h"

void TBDebugOut(const char *str)
{
	print_verbose(str);
}

#endif // TB_RUNTIME_DEBUG_INFO

namespace tb {

// == TBSystem ========================================

double TBSystem::GetTimeMS()
{
	return OS::get_singleton()->get_time_ms();
}

// void TBSystem::RescheduleTimer(double fire_time) { }

int TBSystem::GetLongClickDelayMS()
{
	return 500;
}

int TBSystem::GetPanThreshold()
{
	return 5 * GetDPI() / 96;
}

int TBSystem::GetPixelsPerLine()
{
	return 40 * GetDPI() / 96;
}

int TBSystem::GetDPI()
{
	return 96;
}

} // namespace tb

#endif // TB_SYSTEM_GODOT
