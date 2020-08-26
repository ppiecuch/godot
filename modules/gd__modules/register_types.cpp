#include "register_types.h"

#include "core/engine.h"
#include "editor/editor_node.h"

#include "gd/timer2.h"
#include "gd/tween2.h"
#include "gd/inputstorage.h"
#include "gd/trail_2d.h"
#include "gd/phantom.h"
#include "gd/line_builder_2d.h"
#include "gd/byteswap.h"
#include "gd/tags.h"
#include "gd/blitter.h"
#include "gd/godoterrorhandler.h"
#include "gd/cripter.h"
#include "gd/procedural_animation.h"
#include "gd/procedural_animation_editor_plugin.h"
#include "gd/bs_input_event_key.h"
#include "gd/input_map_editor.h"

#include "gd2c/bytecode_exporter.h"
#include "gd2c/gd2c.h"

#include "statemachine/statemachine.h"
#include "statemachine/state.h"

#include "debugdraw/debugdraw.h"

#include "scene/autotilemap.h"
#include "scene/bullet_manager.h"
#include "scene/pixel_spaceships.h"
#include "scene/vegetation_instance.h"
#include "scene/cable2d.h"
#include "scene/cable2d_editor_plugin.h"
#include "scene/touch_button.h"
#include "scene/audio_stream_preview_generator.h"

#include "benet/enet_packet_peer.h"
#include "benet/enet_node.h"


static bool enet_ok = false;

#ifdef TOOLS_ENABLED
static void editor_init_callback() {
    EditorNode::get_singleton()->add_editor_plugin(memnew(Cable2DEditorPlugin(EditorNode::get_singleton())));
}
#endif

void register_gd__modules_types() {
	ClassDB::register_class<Timer2>();
	ClassDB::register_class<TimerObject>();
	ClassDB::register_class<Tween2>();
	ClassDB::register_class<TweenAction>();
	ClassDB::register_class<InputStorage>();
	ClassDB::register_class<InputStorageNode>();
	ClassDB::register_class<TrailPoint2D>();
	ClassDB::register_class<TrailLine2D>();
	ClassDB::register_class<LineBuilder2D>();
	ClassDB::register_class<Phantom>();
	ClassDB::register_class<Byteswap>();
#ifdef MODULE_MBEDTLS_ENABLED
	ClassDB::register_class<Cripter>();
#endif
	ClassDB::register_class<ProceduralAnimation>();
#ifdef TOOLS_ENABLED
	EditorPlugins::add_by_type<ProceduralAnimationEditorPlugin>();
#endif
	ClassDB::register_class<BSInputEventKey>();
	ClassDB::register_class<InputMapEditor>();

	ClassDB::register_class<GDScriptBytecodeExporter>();
	ClassDB::register_class<GD2CApi>();

	Engine::get_singleton()->add_singleton(Engine::Singleton("Timer2", memnew(Timer2)));
	Engine::get_singleton()->add_singleton(Engine::Singleton("Tween2", memnew(Tween2)));
	Engine::get_singleton()->add_singleton(Engine::Singleton("InputStorage", memnew(InputStorage)));

	ClassDB::register_class<StateMachine>();
	ClassDB::register_class<State>();

	Engine::get_singleton()->add_singleton(Engine::Singleton("DebugDraw", memnew(DebugDraw)));
    Engine::get_singleton()->add_singleton(Engine::Singleton("Tags", memnew(Tags)));
	Engine::get_singleton()->add_singleton(Engine::Singleton("Blitter", memnew(_Blitter)));
	Engine::get_singleton()->add_singleton(Engine::Singleton("GodotErrorHandler", memnew(GodotErrorHandler)));

	ClassDB::register_class<PixelSpaceshipsOptions>();
	ClassDB::register_class<PixelSpaceshipsMask>();
	ClassDB::register_class<PixelSpaceships>();
	ClassDB::register_class<BulletManagerBulletType>();
	ClassDB::register_class<BulletManager>();
	ClassDB::register_class<Autotilemap>();
	ClassDB::register_class<VegetationInstance>();
	ClassDB::register_class<Cable2D>();
#ifdef TOOLS_ENABLED
    EditorNode::add_init_callback(editor_init_callback);
#endif
	ClassDB::register_class<TouchButton>();
	ClassDB::register_class<AudioStreamPreviewGenerator>();

	if (enet_initialize() != 0 ) {
		ERR_PRINT( "ENet initialization failure" );
	} else {
		enet_ok = true;
	}

	ClassDB::register_class<ENetPacketPeer>();
	ClassDB::register_class<ENetNode>();
}

void unregister_gd__modules_types() {
	if (enet_ok)
		enet_deinitialize();
}
