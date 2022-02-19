/*************************************************************************/
/*  register_types.cpp                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
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
#include "editor/audio_stream_preview.h"
#include "editor/editor_node.h"

#include "core/area_prober.h"
#include "core/blitter.h"
#include "core/bs_input_event_key.h"
#include "core/byteswap.h"
#ifdef MODULE_MBEDTLS_ENABLED
#include "core/cripter.h"
#endif
#include "core/dist_rand.h"
#include "core/error_reporter.h"
#include "core/godot_error_handler.h"
#include "core/input_map_editor.h"
#include "core/input_storage.h"
#include "core/line_builder_2d.h"
#include "core/phantom.h"
#include "core/procedural_animation.h"
#include "core/procedural_animation_editor_plugin.h"
#include "core/raw_packer.h"
#include "core/resource_importer_json.h"
#include "core/resources_config.h"
#include "core/tags.h"
#include "core/timer2.h"
#include "core/trail_2d.h"
#include "core/tween2.h"
#include "core/voronoi.h"

#include "bulletkit/gdlibrary.h"

#include "statemachine/state.h"
#include "statemachine/statemachine.h"

#include "behaviornode/behaviornode.h"
#include "behaviornode/linkerbnode.h"
#include "behaviornode/probabilitybnode.h"
#include "behaviornode/statusbnode.h"
#include "behaviornode/timebnode.h"

#include "debugdraw/debugdraw.h"

#include "landiscovery/lan.h"

#include "visual/autotilemap.h"
#include "visual/bend_deform_2d.h"
#include "visual/bullet_manager.h"
#include "visual/cable_2d.h"
#include "visual/cable_2d_editor_plugin.h"
#include "visual/destructible_sprite.h"
#include "visual/explosion_particles.h"
#include "visual/figure_2d.h"
#include "visual/nixie_font.h"
#include "visual/pixel_spaceships.h"
#include "visual/round_progress.h"
#include "visual/sprite_mesh.h"
#include "visual/texture_panning.h"
#include "visual/touch_button.h"

#include "vgamepad/vgamepad.h"

#include "environment/spherical_waves/spherical_waves.h"
#include "environment/spider_anim/spider.h"
#include "environment/spider_anim/stage.h"
#include "environment/starfield/starfield_2d.h"
#include "environment/tree_2d/tree_2d.h"
#include "environment/vegetation_instance/vegetation_instance.h"
#include "environment/water_splash/gd_water_splash.h"

#include "settings/settings.h"

#include "smooth/smooth.h"
#include "smooth/smooth_2d.h"

#include "benet/enet_node.h"
#include "benet/enet_packet_peer.h"

#include "sfxr/gdsfxr.h"

#if defined(OSX_ENABLED) || defined(UWP_ENABLED) || defined (IPHONE_ENABLED)
#include "iap/gd_iap.h"
#endif

#ifdef TOOLS_ENABLED
#include "qrcodetexture/qrcodetexture.h"
#endif

#ifndef _3D_DISABLED
#include "ccd/gd_ccd.h"
#endif

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
	register_bullet_kit();

	ClassDB::register_class<RealNormal>();
	ClassDB::register_class<IntNormal>();

	ClassDB::register_class<Voronoi>();
	ClassDB::register_class<VoronoiDiagram>();
	ClassDB::register_class<VoronoiSite>();
	ClassDB::register_class<VoronoiEdge>();

	ClassDB::register_class<BehaviorNode>();
	ClassDB::register_class<TimerBNode>();
	ClassDB::register_class<ProbabilityBNode>();
	ClassDB::register_class<LinkerBNode>();
	ClassDB::register_class<StatusBNode>();

	ClassDB::register_class<AreaProber>();
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
	ClassDB::register_virtual_class<RawPacker>();
#ifdef MODULE_MBEDTLS_ENABLED
	ClassDB::register_class<Cripter>();
#endif
	ClassDB::register_class<ProceduralAnimation>();
	ClassDB::register_class<BSInputEventKey>();
#ifndef ADVANCED_GUI_DISABLED
	ClassDB::register_class<InputMapEditor>();
#endif
	ClassDB::register_class<StateMachine>();
	ClassDB::register_class<State>();

	ClassDB::register_class<LanAdvertiser>();
	ClassDB::register_class<LanListener>();
	ClassDB::register_class<LanPlayer>();

	Engine::get_singleton()->add_singleton(Engine::Singleton("Resources", memnew(Resources)));
	Engine::get_singleton()->add_singleton(Engine::Singleton("Timer2", memnew(Timer2)));
	Engine::get_singleton()->add_singleton(Engine::Singleton("Tween2", memnew(Tween2)));
	Engine::get_singleton()->add_singleton(Engine::Singleton("InputStorage", memnew(InputStorage)));
	Engine::get_singleton()->add_singleton(Engine::Singleton("DebugDraw", memnew(DebugDraw)));
	Engine::get_singleton()->add_singleton(Engine::Singleton("Tags", memnew(Tags)));
	Engine::get_singleton()->add_singleton(Engine::Singleton("Blitter", memnew(Blitter)));
	Engine::get_singleton()->add_singleton(Engine::Singleton("Settings", memnew(Settings)));
#if defined(OSX_ENABLED) || defined(UWP_ENABLED) || defined (IPHONE_ENABLED)
	ClassDB::register_class<GdInAppStore>();
	ClassDB::register_class<GdInAppStoreInstance>();
#endif

	ClassDB::register_class<JSONData>();
#ifdef TOOLS_ENABLED
	Ref<ResourceImporterJSON> json_data = memnew(ResourceImporterJSON);
	ResourceFormatImporter::get_singleton()->add_importer(json_data);
#endif

	ClassDB::register_class<AudioStreamSfxr>();
	ClassDB::register_class<AudioStreamPlaybackSfxr>();
#ifdef TOOLS_ENABLED
	Ref<ResourceImporterSfxr> sfx_data = memnew(ResourceImporterSfxr);
	ResourceFormatImporter::get_singleton()->add_importer(sfx_data);
#endif

	ClassDB::register_class<PixelSpaceshipsOptions>();
	ClassDB::register_class<PixelSpaceshipsMask>();
	ClassDB::register_class<PixelSpaceships>();
	ClassDB::register_class<BulletManagerBulletType>();
	ClassDB::register_class<BulletManager>();
	ClassDB::register_class<Autotilemap>();
	ClassDB::register_class<ElasticSimulation>();
	ClassDB::register_class<SimulationController2D>();
	ClassDB::register_class<SimulationControllerInstance2D>();
	ClassDB::register_class<ElasticSprite>();
	ClassDB::register_class<ElasticMeshInstance2D>();
	ClassDB::register_class<DestructibleSprite>();
	ClassDB::register_class<Cable2D>();
	ClassDB::register_class<TouchButton>();
	ClassDB::register_class<VGamePad>();
#ifdef TOOLS_ENABLED
	ClassDB::register_class<AudioStreamPreview>();
	ClassDB::register_class<AudioStreamPreviewGenerator>();
#endif
	ClassDB::register_class<SpriteMesh>();
	ClassDB::register_class<Figure2D>();

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
	ClassDB::register_class<SpiderStage>();
	ClassDB::register_class<SpiderStageInstance>();

#ifdef MODULE_ENET_ENABLED
	ClassDB::register_class<ENetPacketPeer>();
	ClassDB::register_class<ENetNode>();
#endif

	ClassDB::register_class<Smooth>();
	ClassDB::register_class<Smooth2D>();

#ifdef TOOLS_ENABLED
	ClassDB::register_class<QRCodeTexture>();
	ClassDB::register_class<ErrorReporter>();
#endif

#ifndef _3D_DISABLED
	ClassDB::register_class<CCDBox>();
	ClassDB::register_class<CCDSphere>();
	ClassDB::register_class<CCDCylinder>();
#endif

#ifdef TOOLS_ENABLED
	EditorNode::add_init_callback(editor_init_callback);
#endif
}

void unregister_gdextensions_types() {
	if (Resources *instance = Resources::get_singleton()) {
		memdelete(instance);
	}
	if (Timer2 *instance = Timer2::get_singleton()) {
		memdelete(instance);
	}
	if (Tween2 *instance = Tween2::get_singleton()) {
		memdelete(instance);
	}
	if (InputStorage *instance = InputStorage::get_singleton()) {
		memdelete(instance);
	}
	if (DebugDraw *instance = DebugDraw::get_singleton()) {
		memdelete(instance);
	}
	if (Tags *instance = Tags::get_singleton()) {
		memdelete(instance);
	}
	if (Blitter *instance = Blitter::get_singleton()) {
		memdelete(instance);
	}
	if (Settings *instance = Settings::get_singleton()) {
		memdelete(instance);
	}
#if defined(OSX_ENABLED) || defined(UWP_ENABLED) || defined (IPHONE_ENABLED)
	GdInAppStore::release_store();
#endif
#ifdef TOOLS_ENABLED
	if (GodotErrorHandler *instance = GodotErrorHandler::get_singleton()) {
		memdelete(instance);
	}
#endif
}
