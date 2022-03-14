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
#include "core/io/resource_importer.h"
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

#include "blitter/gd_bitblit.h"

#include "statemachine/state.h"
#include "statemachine/statemachine.h"

#ifdef GDEXT_BEHAVIORNODE_ENABLED
#include "behaviornode/behaviornode.h"
#include "behaviornode/linkerbnode.h"
#include "behaviornode/probabilitybnode.h"
#include "behaviornode/statusbnode.h"
#include "behaviornode/timebnode.h"
#endif

#ifdef GDEXT_BEHAVIORTREE_ENABLED
#include "behaviortree/bt_action_node.h"
#include "behaviortree/bt_composite_node.h"
#include "behaviortree/bt_custom_parallel_node.h"
#include "behaviortree/bt_decorator_node.h"
#include "behaviortree/bt_root_node.h"
#include "behaviortree/bt_string_names.h"
#endif

#include "debugdraw/debugdraw.h"

#include "landiscovery/lan.h"

#include "generator/gd_procedural_mesh.h"

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

#include "threadpool/thread_pool.h"
#include "threadpool/thread_pool_execute_job.h"
#include "threadpool/thread_pool_job.h"

#include "fastnoise/noise.h"

#include "geomfonts//gd_geomfonts.h"

#if defined(OSX_ENABLED) || defined(UWP_ENABLED) || defined(IPHONE_ENABLED) || defined(ANDROID_ENABLED)
#include "iap/gd_iap.h"
#endif

#ifdef GDEXT_QRCODETEXTURE_ENABLED
#ifdef TOOLS_ENABLED
#include "qrcodetexture/qrcodetexture.h"
#endif
#endif

#ifndef _3D_DISABLED
#include "ccd/gd_ccd.h"
#endif

#ifdef GDEXT_NAKAMA1_ENABLED
#include "nakama1/gd_nakama1.h"
#endif

#ifdef GDEXT_PARSEPLATFORM_ENABLED
#include "parseplatform/gd_parse_platform.h"
#endif

#ifdef GDEXT_FLEXBUFFERS_ENABLED
#include "flexbuffers/resource_importer_flexbuffer.h"
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

static ThreadPool *thread_pool = nullptr;

void register_gdextensions_types() {
#ifdef GDEXT_BULLETKIT_ENABLED
	register_bullet_kit();
#endif
#ifdef GDEXT_BLITTER_ENABLED
	ClassDB::register_virtual_class<BlitSurface>();
	Engine::get_singleton()->add_singleton(Engine::Singleton("BitBlit", memnew(BitBlit)));
#endif
#ifdef GDEXT_BEHAVIORNODE_ENABLED
	ClassDB::register_class<BehaviorNode>();
	ClassDB::register_class<TimerBNode>();
	ClassDB::register_class<ProbabilityBNode>();
	ClassDB::register_class<LinkerBNode>();
	ClassDB::register_class<StatusBNode>();
#endif
#ifdef GDEXT_BEHAVIORTREE_ENABLED
	BTStringNames::create();
	ClassDB::register_class<BTRootNode>();
	ClassDB::register_class<BTActionNode>();
	ClassDB::register_class<BTDecoratorNode>();
	ClassDB::register_class<BTSequenceNode>();
	ClassDB::register_class<BTSelectorNode>();
	ClassDB::register_class<BTParallelNode>();
	ClassDB::register_class<BTCustomParallelNode>();
#endif
#ifdef GDEXT_CORE_ENABLED
	ClassDB::register_class<AreaProber>();
	ClassDB::register_class<BSInputEventKey>();
	ClassDB::register_class<Byteswap>();
#ifdef MODULE_MBEDTLS_ENABLED
	ClassDB::register_class<Cripter>();
#endif
#ifdef TOOLS_ENABLED
	ClassDB::register_class<ErrorReporter>();
#endif
#ifndef ADVANCED_GUI_DISABLED
	ClassDB::register_class<InputMapEditor>();
#endif
	ClassDB::register_class<InputStorage>();
	ClassDB::register_class<InputStorageNode>();
	ClassDB::register_class<IntNormal>();
	ClassDB::register_class<JSONData>();
#ifdef TOOLS_ENABLED
	Ref<ResourceImporterJSON> json_data = memnew(ResourceImporterJSON);
	ResourceFormatImporter::get_singleton()->add_importer(json_data);
#endif
#ifdef GDEXT_THREADPOOL_ENABLED
	ClassDB::register_class<ThreadPoolJob>();
	ClassDB::register_class<ThreadPoolExecuteJob>();

	thread_pool = memnew(ThreadPool);
	ClassDB::register_class<ThreadPool>();
	Engine::get_singleton()->add_singleton(Engine::Singleton("ThreadPool", ThreadPool::get_singleton()));
#endif
	ClassDB::register_class<LineBuilder2D>();
	ClassDB::register_class<Phantom>();
	ClassDB::register_virtual_class<RawPacker>();
	ClassDB::register_class<RealNormal>();
	ClassDB::register_class<RoundProgress>();
	ClassDB::register_class<Timer2>();
	ClassDB::register_class<TimerObject>();
	ClassDB::register_class<Tween2>();
	ClassDB::register_class<TweenAction>();
	ClassDB::register_class<TrailPoint2D>();
	ClassDB::register_class<TrailLine2D>();
	ClassDB::register_class<ProceduralAnimation>();
	ClassDB::register_class<Voronoi>();
	ClassDB::register_class<VoronoiDiagram>();
	ClassDB::register_class<VoronoiSite>();
	ClassDB::register_class<VoronoiEdge>();
#endif // GDEXT_CORE_ENABLED
#ifdef GDEXT_STATEMACHINE_ENABLED
	ClassDB::register_class<StateMachine>();
	ClassDB::register_class<State>();
#endif // GDEXT_STATEMACHINE_ENABLED
#ifdef GDEXT_LANADVERTISER_ENABLED
	ClassDB::register_class<LanAdvertiser>();
	ClassDB::register_class<LanListener>();
	ClassDB::register_class<LanPlayer>();
#endif // GDEXT_LANADVERTISER_ENABLED
#ifdef GDEXT_CORE_ENABLED
	Engine::get_singleton()->add_singleton(Engine::Singleton("Resources", memnew(Resources)));
	Engine::get_singleton()->add_singleton(Engine::Singleton("Timer2", memnew(Timer2)));
	Engine::get_singleton()->add_singleton(Engine::Singleton("Tween2", memnew(Tween2)));
	Engine::get_singleton()->add_singleton(Engine::Singleton("InputStorage", memnew(InputStorage)));
	Engine::get_singleton()->add_singleton(Engine::Singleton("Tags", memnew(Tags)));
	Engine::get_singleton()->add_singleton(Engine::Singleton("Blitter", memnew(Blitter)));
#endif // GDEXT_CORE_ENABLED
#ifdef GDEXT_SETTINGS_ENABLED
	Engine::get_singleton()->add_singleton(Engine::Singleton("Settings", memnew(Settings)));
#endif
#ifdef GDEXT_DEBUGDRAW_ENABLED
	Engine::get_singleton()->add_singleton(Engine::Singleton("DebugDraw", memnew(DebugDraw)));
#endif
#ifdef GDEXT_IAP_ENABLED
#if defined(OSX_ENABLED) || defined(UWP_ENABLED) || defined(IPHONE_ENABLED) || defined(ANDROID_ENABLED)
	register_iap_platform();
#endif
#endif // GDEXT_IAP_ENABLED
#ifdef GDEXT_SFXR_ENABLED
	ClassDB::register_class<AudioStreamSfxr>();
	ClassDB::register_class<AudioStreamPlaybackSfxr>();
#ifdef TOOLS_ENABLED
	Ref<ResourceImporterSfxr> sfx_data = memnew(ResourceImporterSfxr);
	ResourceFormatImporter::get_singleton()->add_importer(sfx_data);
#endif
#endif // GDEXT_SFXR_ENABLED

#ifdef GDEXT_GEOMFONTS_ENABLED
	ClassDB::register_class<GdGeomFonts>();
#endif

#ifdef GDEXT_GENERATOR_ENABLED
	ClassDB::register_class<ProceduralMesh>();
#endif

#ifdef GDEXT_FASTNOISE_ENABLED
	ClassDB::register_class<FastNoise>();
#endif

#ifdef GDEXT_VISUAL_ENABLED
	ClassDB::register_class<Autotilemap>();
	ClassDB::register_class<BulletManagerBulletType>();
	ClassDB::register_class<BulletManager>();
	ClassDB::register_class<Cable2D>();
	ClassDB::register_class<DestructibleSprite>();
	ClassDB::register_class<ElasticSimulation>();
	ClassDB::register_class<ElasticSprite>();
	ClassDB::register_class<ElasticMeshInstance2D>();
	ClassDB::register_class<FakeExplosionParticles2D>();
	ClassDB::register_class<Figure2D>();
	ClassDB::register_class<NixieFont>();
	ClassDB::register_class<PixelSpaceshipsOptions>();
	ClassDB::register_class<PixelSpaceshipsMask>();
	ClassDB::register_class<PixelSpaceships>();
	ClassDB::register_class<SimulationController2D>();
	ClassDB::register_class<SimulationControllerInstance2D>();
	ClassDB::register_class<SphericalWaves>();
	ClassDB::register_class<SpriteMesh>();
	ClassDB::register_class<Starfield2D>();
	ClassDB::register_class<TouchButton>();
	ClassDB::register_class<TexturePanning>();
#endif // GDEXT_VISUAL_ENABLED
#ifdef GDEXT_VGAMEPAD_ENABLED
	ClassDB::register_class<VGamePad>();
#endif // GDEXT_VGAMEPAD_ENABLED
#ifdef TOOLS_ENABLED
	ClassDB::register_class<AudioStreamPreview>();
	ClassDB::register_class<AudioStreamPreviewGenerator>();
#endif

#ifdef GDEXT_ENVIRONMENT_ENABLED
	ClassDB::register_class<Tree2D>();
	ClassDB::register_class<GDWaterSplash>();
	ClassDB::register_virtual_class<GDWaterSplashColumn>();
#ifndef _3D_DISABLED
	ClassDB::register_class<VegetationInstance>();
#endif
	ClassDB::register_class<Spider>();
	ClassDB::register_class<SpiderStage>();
	ClassDB::register_class<SpiderStageInstance>();
#endif // GDEXT_ENVIRONMENT_ENABLED

#ifdef GDEXT_BENET_ENABLED
#ifdef MODULE_ENET_ENABLED
	ClassDB::register_class<ENetPacketPeer>();
	ClassDB::register_class<ENetNode>();
#endif
#endif // GDEXT_BENET_ENABLED

#ifdef GDEXT_FLEXBUFFERS_ENABLED
	ClassDB::register_class<FlexbuffersData>();
	Ref<ResourceImporterFlexbuffers> flexbuffers_data;
	flexbuffers_data.instance();
	ResourceFormatImporter::get_singleton()->add_importer(flexbuffers_data);
#endif

#ifdef GDEXT_NAKAMA1_ENABLED
	ClassDB::register_virtual_class<NkCollatedMessage>();
	ClassDB::register_virtual_class<NkUncollatedMessage>();
	Engine::get_singleton()->add_singleton(Engine::Singleton("GdNakama1", memnew(GdNakama1)));
#endif

#ifdef GDEXT_PARSEPLATFORM_ENABLED
	print_line(vformat("ParsePlatform Godot module: %d.%d.%d", GODOTPARSE_MAJOR, GODOTPARSE_MINOR, GODOTPARSE_PATCH));

	Engine::get_singleton()->add_singleton(Engine::Singleton("GdParseBackend", memnew(GdParseBackend)));
	ClassDB::register_virtual_class<GdParseError>();
	ClassDB::register_virtual_class<GdParseQuery>();
	ClassDB::register_virtual_class<GdParseObject>();
#endif

#ifdef GDEXT_SMOOTH_ENABLED
	ClassDB::register_class<Smooth>();
	ClassDB::register_class<Smooth2D>();
#endif // GDEXT_SMOOTH_ENABLED

#ifdef GDEXT_QRCODETEXTURE_ENABLED
#ifdef TOOLS_ENABLED
	ClassDB::register_class<QRCodeTexture>();
#endif // TOOLS_ENABLED
#endif // GDEXT_QRCODETEXTURE_ENABLED

#ifdef GDEXT_CCD_ENABLED
#ifndef _3D_DISABLED
	ClassDB::register_class<CCDBox>();
	ClassDB::register_class<CCDSphere>();
	ClassDB::register_class<CCDCylinder>();
#endif // _3D_DISABLED
#endif // GDEXT_CCD_ENABLED

#ifdef TOOLS_ENABLED
	EditorNode::add_init_callback(editor_init_callback);
#endif
}

void unregister_gdextensions_types() {
	if (thread_pool) {
		memdelete(thread_pool);
	}
#ifdef GDEXT_BEHAVIORTREE_ENABLED
	BTStringNames::free();
#endif
#ifdef GDEXT_BLITTER_ENABLED
	if (BitBlit *instance = BitBlit::get_singleton()) {
		memdelete(instance);
	}
#endif
#ifdef GDEXT_CORE_ENABLED
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
	if (Tags *instance = Tags::get_singleton()) {
		memdelete(instance);
	}
	if (Blitter *instance = Blitter::get_singleton()) {
		memdelete(instance);
	}
#ifdef TOOLS_ENABLED
	if (GodotErrorHandler *instance = GodotErrorHandler::get_singleton()) {
		memdelete(instance);
	}
#endif
#endif // GDEXT_CORE_ENABLED
#ifdef GDEXT_DEBUGDRAW_ENABLED
	if (DebugDraw *instance = DebugDraw::get_singleton()) {
		memdelete(instance);
	}
#endif
#ifdef GDEXT_SETTINGS_ENABLED
	if (Settings *instance = Settings::get_singleton()) {
		memdelete(instance);
	}
#endif
#ifdef GDEXT_NAKAMA1_ENABLED
	if (GdNakama1 *instance = GdNakama1::get_singleton()) {
		memdelete(instance);
	}
#endif
#ifdef GDEXT_PARSEPLATFORM_ENABLED
	if (GdParseBackend *instance = GdParseBackend::get_singleton()) {
		memdelete(instance);
	}
#endif
#ifdef GDEXT_IAP_ENABLED
#if defined(OSX_ENABLED) || defined(UWP_ENABLED) || defined(IPHONE_ENABLED)
	GdInAppStore::release_store();
#endif
#endif // GDEXT_IAP_ENABLED
}
