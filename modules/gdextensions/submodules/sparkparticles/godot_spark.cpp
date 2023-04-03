/**************************************************************************/
/*  godot_spark.cpp                                                       */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "core/engine.h"
#include "core/os/file_access.h"

#include "godot_spark.h"

void GdSparkParticles::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			if (!Engine::get_singleton()->is_editor_hint()) {
				set_process_input(is_visible_in_tree());
			}
		} break;
		case NOTIFICATION_VISIBILITY_CHANGED: {
			if (Engine::get_singleton()->is_editor_hint()) {
				break;
			}
			if (is_visible_in_tree()) {
				set_process_input(true);
			} else {
				set_process_input(false);
			}
		} break;
		case NOTIFICATION_READY: {
		} break;
		case NOTIFICATION_DRAW: {
			if (system && renderer) {
				system->renderParticles();
				renderer->flushRender();
			}
		} break;
		case NOTIFICATION_PROCESS: {
		} break;
	}
}

Error GdSparkParticles::load_from_file(const String &p_path) {
	ERR_FAIL_COND_V(!FileAccess::exists(p_path), ERR_FILE_NOT_FOUND);
	SPK::Ref<SPK::System> system2 = SPK::IO::IOManager::get().load(p_path.utf8().c_str());
	return OK;
}

void GdSparkParticles::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load_from_file"), &GdSparkParticles::load_from_file);
}

GdSparkParticles::GdSparkParticles() {
	_system = SPK::System::create(true);
	_system->setName("Spark Particles");
	renderer_type = SPK_QUAD_REDERER;
}
