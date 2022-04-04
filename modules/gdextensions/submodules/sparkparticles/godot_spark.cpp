#include "core/engine.h"

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
	}
}

void GdSparkParticles::_bind_methods() {
}

GdSparkParticles::GdSparkParticles() {
	_system = SPK::System::create(true);
	_system->setName("Spark Particles");
	renderer_type = SPK_QUAD_REDERER;
}
