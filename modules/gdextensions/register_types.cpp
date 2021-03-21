/*************************************************************************/
/*  register_types.cpp                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "register_types.h"

#include "core/engine.h"
#include "editor/editor_node.h"
#include "editor/audio_stream_preview.h"

#include "gd/blitter.h"
#include "gd/bs_input_event_key.h"
#include "gd/byteswap.h"
#include "gd/cripter.h"
#include "gd/godoterrorhandler.h"
#include "gd/input_map_editor.h"
#include "gd/inputstorage.h"
#include "gd/line_builder_2d.h"
#include "gd/phantom.h"
#include "gd/procedural_animation.h"
#include "gd/procedural_animation_editor_plugin.h"
#include "gd/tags.h"
#include "gd/timer2.h"
#include "gd/trail_2d.h"
#include "gd/tween2.h"

#include "gd2c/bytecode_exporter.h"
#include "gd2c/gd2c.h"

#include "statemachine/state.h"
#include "statemachine/statemachine.h"

#include "behaviornode/behaviornode.h"
#include "behaviornode/linkerbnode.h"
#include "behaviornode/probabilitybnode.h"
#include "behaviornode/statusbnode.h"
#include "behaviornode/timebnode.h"

#include "debugdraw/debugdraw.h"

#include "scene/autotilemap.h"
#include "scene/bend_deform_2d.h"
#include "scene/bullet_manager.h"
#include "scene/cable2d.h"
#include "scene/cable2d_editor_plugin.h"
#include "scene/explosion_particles.h"
#include "scene/figure.h"
#include "scene/nixie_font.h"
#include "scene/pixel_spaceships.h"
#include "scene/round_progress.h"
#include "scene/sprite_mesh.h"
#include "scene/texture_panning.h"
#include "scene/touch_button.h"

#include "scenery/spherical_waves/spherical_waves.h"
#include "scenery/spider_anim/spider.h"
#include "scenery/spider_anim/spider_insects.h"
#include "scenery/starfield/starfield_2d.h"
#include "scenery/tree_2d/tree_2d.h"
#include "scenery/vegetation_instance/vegetation_instance.h"
#include "scenery/water_splash/gd_water_splash.h"

#include "smooth/smooth.h"
#include "smooth/smooth_2d.h"

#include "benet/enet_node.h"
#include "benet/enet_packet_peer.h"

static Vector<Object *> _global_resources;
void _register_global_resources(Object *ref) {
	_global_resources.push_back(ref);
}

#ifdef TOOLS_ENABLED
static void editor_init_callback() {

	Engine::get_singleton()->add_singleton(Engine::Singleton("GodotErrorHandler", memnew(GodotErrorHandler)));

	EditorNode::get_singleton()->add_editor_plugin(memnew(Cable2DEditorPlugin(EditorNode::get_singleton()))); /* Cable2D */

	EditorPlugins::add_by_type<ProceduralAnimationEditorPlugin>(); /* ProceduralAnimation */
}
#endif

void register_gdextensions_types() {

	ClassDB::register_class<BehaviorNode>();
	ClassDB::register_class<TimerBNode>();
	ClassDB::register_class<ProbabilityBNode>();
	ClassDB::register_class<LinkerBNode>();
	ClassDB::register_class<StatusBNode>();

	ClassDB::register_class<Timer2>();
	ClassDB::register_class<TimerObject>();
	ClassDB::register_class<Tween2>();
	ClassDB::register_class<TweenAction>();
	ClassDB::register_class<InputStorage>();
	ClassDB::register_class<InputStorageNode>();
	ClassDB::register_class<TrailPoint2D>();
	ClassDB::register_class<TrailLine2D>();
	ClassDB::register_class<LineBuilder2D>();
	ClassDB::register_class<RoundProgress>();
	ClassDB::register_class<Phantom>();
	ClassDB::register_class<Byteswap>();
#ifdef MODULE_MBEDTLS_ENABLED
	ClassDB::register_class<Cripter>();
#endif
	ClassDB::register_class<ProceduralAnimation>();
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
	Engine::get_singleton()->add_singleton(Engine::Singleton("Blitter", memnew(Blitter)));

	ClassDB::register_class<PixelSpaceshipsOptions>();
	ClassDB::register_class<PixelSpaceshipsMask>();
	ClassDB::register_class<PixelSpaceships>();
	ClassDB::register_class<BulletManagerBulletType>();
	ClassDB::register_class<BulletManager>();
	ClassDB::register_class<Autotilemap>();
	ClassDB::register_class<ElasticSimulation>();
	ClassDB::register_class<SimulationController2D>();
	ClassDB::register_class<SimulationControllerInstance2D>();
	ClassDB::register_class<DeformMeshInstance2D>();
	ClassDB::register_class<DeformSprite>();
	ClassDB::register_class<Cable2D>();
	ClassDB::register_class<TouchButton>();
#ifdef TOOLS_ENABLED
	ClassDB::register_class<AudioStreamPreview>();
	ClassDB::register_class<AudioStreamPreviewGenerator>();
#endif
	ClassDB::register_class<SpriteMesh>();
	ClassDB::register_class<Figure>();

	ClassDB::register_class<Tree2D>();
	ClassDB::register_class<GDWaterSplash>();
	ClassDB::register_virtual_class<GDWaterSplashColumn>();
#ifndef _3D_DISABLED
	ClassDB::register_class<VegetationInstance>();
#endif
	ClassDB::register_class<SphericalWaves>();
	ClassDB::register_class<Starfield2D>();
	ClassDB::register_class<TexturePanning>();
	ClassDB::register_class<FakeExplosionParticles2D>();
	ClassDB::register_class<NixieFont>();
	ClassDB::register_class<Spider>();
	ClassDB::register_virtual_class<InsectsManager>();
	ClassDB::register_class<InsectsManagerInstance>();

#ifdef MODULE_ENET_ENABLED
	ClassDB::register_class<ENetPacketPeer>();
	ClassDB::register_class<ENetNode>();
#endif

	ClassDB::register_class<Smooth>();
	ClassDB::register_class<Smooth2D>();

#ifdef TOOLS_ENABLED
	EditorNode::add_init_callback(editor_init_callback);
#endif
}

void unregister_gdextensions_types() {

	if (Timer2 *instance = Timer2::get_singleton())
		memdelete(instance);
	if (Tween2 *instance = Tween2::get_singleton())
		memdelete(instance);
	if (InputStorage *instance = InputStorage::get_singleton())
		memdelete(instance);
	if (DebugDraw *instance = DebugDraw::get_singleton())
		memdelete(instance);
	if (Tags *instance = Tags::get_singleton())
		memdelete(instance);
	if (Blitter *instance = Blitter::get_singleton())
		memdelete(instance);
#ifdef TOOLS_ENABLED
	if (GodotErrorHandler *instance = GodotErrorHandler::get_singleton())
		memdelete(instance);
#endif
}
