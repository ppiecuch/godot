/**************************************************************************/
/*  register_types.cpp                                                    */
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

#include "register_types.h"

#include "core/engine.h"
#include "core/io/resource_importer.h"

#ifdef TOOLS_ENABLED
#include "editor/audio_stream_preview.h"
#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#endif

#include "common/resources_cache.h"
#include "common/resources_config.h"
#include "common/sr_graph.h"

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
#include "core/stopwatch.h"
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

#ifdef GDEXT_MULTIPEER_ENABLED
#include "multipeer/gd_multipeer.h"
#endif

#ifdef GDEXT_HTTPSERVER_ENABLED
#include "httpserver/gd_http_server.h"
#endif

#ifdef GDEXT_GENERATOR_ENABLED
#include "generator/gd_procedural_mesh.h"
#endif

#include "visual/autotilemap.h"
#include "visual/bend_deform_2d.h"
#include "visual/bullet_manager.h"
#include "visual/cable_2d.h"
#include "visual/cable_2d_editor_plugin.h"
#include "visual/destructible_sprite.h"
#include "visual/explosion_particles.h"
#include "visual/figure_2d.h"
#include "visual/grid_rect.h"
#include "visual/nixie_font.h"
#include "visual/pixel_spaceships.h"
#include "visual/round_progress.h"
#include "visual/sprite_mesh.h"
#include "visual/texture_panning.h"
#include "visual/thumb_wheel.h"
#include "visual/touch_button.h"
#include "visual/widget_controls.h"

#include "ropesim/rope_server.h"

#include "vgamepad/vgamepad.h"

#include "environment/spherical_waves/spherical_waves.h"
#include "environment/spider_anim/spider.h"
#include "environment/spider_anim/stage.h"
#include "environment/starfield/starfield_2d.h"
#include "environment/tree_2d/tree_2d.h"
#include "environment/vegetation_instance/vegetation_instance.h"
#include "environment/water_2d/gd_water_2d.h"
#include "environment/water_splash/gd_water_splash.h"
#include "environment/waterfall/gd_waterfall.h"

#ifdef GDEXT_MEDIA_FLAC_ENABLED
#include "media/flac/audio_stream_flac.h"
#ifdef TOOLS_ENABLED
#include "media/flac/resource_importer_flac.h"
#endif
#endif
#ifdef GDEXT_MEDIA_GIFEXPORTER_ENABLED
#include "media/gifexporter/gifexporter.h"
#endif
#ifdef GDEXT_MEDIA_SMACKVIDEO_ENABLED
#include "media/smackvideo/gd_smackvideo.h"
#endif
#ifdef GDEXT_MEDIA_FFMPEG_ENABLED
#include "media/ffmpeg/gd_videodecoder.h"
#endif
#ifdef GDEXT_MEDIA_BMFIMPORTER_ENABLED
#include "media/bmfimporter/bmf_bitmap_font.h"
#endif

#ifdef GDEXT_RUNTIMEPROFILER_ENABLED
#include "runtimeprofiler/runtime_profiler.h"
#endif

#ifdef GDEXT_BENCHMARK_ENABLED
#include "benchmark/benchmark.h"
#endif

#include "settings/settings.h"

#ifdef GDEXT_SQLITE_ENABLED
#include "sqlite/gd_sqlite.h"
#endif
#ifdef GDEXT_UNQLITE_ENABLED
#include "unqlite/gd_unqlite.h"
#endif

#include "smooth/smooth.h"
#include "smooth/smooth_2d.h"

#include "benet/enet_node.h"
#include "benet/enet_packet_peer.h"

#include "sfxr/gdsfxr.h"

#include "threadpool/thread_pool.h"
#include "threadpool/thread_pool_execute_job.h"
#include "threadpool/thread_pool_job.h"

#include "fastnoise/noise.h"
#include "noise/noise.h"

#include "geomfonts//gd_geomfonts.h"

#ifdef GDEXT_IAP_ENABLED
#include "iap/gd_iap.h"
#endif

#ifdef GDEXT_QRCODETEXTURE_ENABLED
#include "qrcodetexture/qrcodetexture.h"
#endif

#ifdef GDEXT_CCD_ENABLED
#include "ccd/gd_ccd.h"
#endif

#ifdef GDEXT_SPINNERS_ENABLED
#include "spinners/gdimspinner.h"
#endif

#ifdef GDEXT_CYBERELEMENTS_ENABLED
#include "cyberelements/cyberelements.h"
#endif

#ifdef GDEXT_POLYVECTOR_ENABLED
#include "polyvector/polyvector.h"
#include "polyvector/resource_importer_swf.h"

static Ref<ResourceLoaderJSONVector> resource_loader_jsonvector;
#endif

#ifdef GDEXT_THORVG_ENABLED
#include "thorvg/image_loader_thor_svg.h"
#include <thorvg.h>

static Ref<ImageLoaderThorSVG> image_loader_tsvg;
#endif

#ifdef GDEXT_MESHLOD_ENABLED
#include "meshlod/optimize.h"
#endif

#ifdef GDEXT_MESHSLICER_ENABLED
#include "meshslicer/slicer.h"
#endif

#ifdef GDEXT_SCENEMERGE_ENABLED
#include "scenemerge/merge.h"
#endif

#ifdef GDEXT_TEXTUREPACKER_ENABLED
#include "texturepacker/packer_image_resource.h"
#include "texturepacker/texture_layer_merger.h"
#include "texturepacker/texture_merger.h"
#include "texturepacker/texture_packer.h"
#ifdef TOOLS_ENABLED
#include "texturepacker/editor_plugin_packer_image_resource.h"
#endif
#endif

#ifdef GDEXT_SILENTWOLF_ENABLED
#include "silentwolf/silent_wolf.h"
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

#ifdef GDEXT_SPACEMOUSE_ENABLED
#include "spacemouse/spacemouse.h"
#endif

#ifdef TOOLS_ENABLED
static void editor_init_callback() {
	Engine::get_singleton()->add_singleton(Engine::Singleton("GodotErrorHandler", memnew(GodotErrorHandler)));

#ifdef GDEXT_VISUAL_ENABLED
	EditorNode::get_singleton()->add_editor_plugin(memnew(Cable2DEditorPlugin(EditorNode::get_singleton()))); /* Cable2D */
	EditorNode::get_singleton()->add_editor_plugin(memnew(SpriteMeshEditorPlugin(EditorNode::get_singleton()))); /* SpriteMesh */
	EditorNode::get_singleton()->add_editor_plugin(memnew(SpriteMeshLightEditorPlugin(EditorNode::get_singleton()))); /* SpriteMeshLight */
#endif

	EditorPlugins::add_by_type<ProceduralAnimationEditorPlugin>(); /* ProceduralAnimation */
}
#endif // TOOLS_ENABLED

static ThreadPool *thread_pool = nullptr;

void register_gdextensions_types() {
	Engine::get_singleton()->add_singleton(Engine::Singleton("ResCache", memnew(ResCache)));
	Engine::get_singleton()->add_singleton(Engine::Singleton("Resources", memnew(Resources)));
	ClassDB::register_class<SRGraph>();
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
	ClassDB::register_class<GridRect>();
	ClassDB::register_class<Timer2>();
	ClassDB::register_class<TimerObject>();
	ClassDB::register_class<Tween2>();
	ClassDB::register_class<TweenAction>();
	ClassDB::register_class<TrailPoint2D>();
	ClassDB::register_class<TrailLine2D>();
	ClassDB::register_class<ProceduralAnimation>();
	ClassDB::register_class<GridRect>();
	ClassDB::register_class<Voronoi>();
	ClassDB::register_class<VoronoiDiagram>();
	ClassDB::register_class<VoronoiSite>();
	ClassDB::register_class<VoronoiEdge>();
#endif // GDEXT_CORE_ENABLED
#ifdef GDEXT_STATEMACHINE_ENABLED
	ClassDB::register_class<StateMachine>();
	ClassDB::register_class<State>();
#endif
#ifdef GDEXT_LANADVERTISER_ENABLED
	ClassDB::register_class<LanAdvertiser>();
	ClassDB::register_class<LanListener>();
	ClassDB::register_class<LanPlayer>();
#endif
#ifdef GDEXT_MULTIPEER_ENABLED
	Engine::get_singleton()->add_singleton(Engine::Singleton("GdMultiPeer", memnew(GdMultiPeer)));
#endif
#ifdef GDEXT_HTTPSERVER_ENABLED
	Engine::get_singleton()->add_singleton(Engine::Singleton("GdHttpServer", memnew(GdHttpServer)));
#endif
#ifdef GDEXT_CORE_ENABLED
	Engine::get_singleton()->add_singleton(Engine::Singleton("Timer2", memnew(Timer2)));
	Engine::get_singleton()->add_singleton(Engine::Singleton("Tween2", memnew(Tween2)));
	Engine::get_singleton()->add_singleton(Engine::Singleton("InputStorage", memnew(InputStorage)));
	Engine::get_singleton()->add_singleton(Engine::Singleton("Tags", memnew(Tags)));
	Engine::get_singleton()->add_singleton(Engine::Singleton("Blitter", memnew(Blitter)));
#endif // GDEXT_CORE_ENABLED
#ifdef GDEXT_SETTINGS_ENABLED
	Engine::get_singleton()->add_singleton(Engine::Singleton("Settings", memnew(Settings)));
#endif
#ifdef GDEXT_SQLITE_ENABLED
	ClassDB::register_class<SQLite>();
#endif
#ifdef GDEXT_UNQLITE_ENABLED
	ClassDB::register_class<UNQLite>();
#endif
#ifdef GDEXT_DEBUGDRAW_ENABLED
	Engine::get_singleton()->add_singleton(Engine::Singleton("DebugDraw", memnew(DebugDraw)));
#endif
#ifdef GDEXT_IAP_ENABLED
	register_iap_platform();
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

#ifdef GDEXT_NOISE_ENABLED
	ClassDB::register_class<NoiseTexture>;
	ClassDB::register_virtual_class<Noise>;
	ClassDB::register_class<FastNoiseLite>;
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
	ClassDB::register_class<SpriteMeshLight>();
	ClassDB::register_class<TouchButton>();
	ClassDB::register_class<TexturePanning>();
	ClassDB::register_class<ThumbWheelH>();
	ClassDB::register_class<ThumbWheelV>();
#ifdef TOOLS_ENABLED
	ClassDB::register_class<ControlWidget>();
#endif
#endif // GDEXT_VISUAL_ENABLED
#ifdef GDEXT_ROPESIM_ENABLED
	ClassDB::register_class<RopeServer>();
#endif
#ifdef GDEXT_SPINNERS_ENABLED
	ClassDB::register_class<Spinner>();
#endif
#ifdef GDEXT_CYBERELEMENTS_ENABLED
	ClassDB::register_class<CyberElement>();
#endif
#ifdef GDEXT_VGAMEPAD_ENABLED
	ClassDB::register_class<VGamePad>();
#endif // GDEXT_VGAMEPAD_ENABLED
#ifdef TOOLS_ENABLED
	ClassDB::register_class<AudioStreamPreview>();
	ClassDB::register_class<AudioStreamPreviewGenerator>();
#endif

#ifdef GDEXT_ENVIRONMENT_WATERFALL_ENABLED
	ClassDB::register_class<GdWaterfall>();
#endif
#ifdef GDEXT_ENVIRONMENT_TREE_2D_ENABLED
	ClassDB::register_class<Tree2D>();
#endif
#ifdef GDEXT_ENVIRONMENT_WATER_SPLASH_ENABLED
	ClassDB::register_class<GdWaterSplash>();
	ClassDB::register_virtual_class<GdWaterSplashColumn>();
#endif
#ifdef GDEXT_ENVIRONMENT_WATER_2D_ENABLED
	ClassDB::register_class<Water2D>();
#endif
#ifndef _3D_DISABLED
#ifdef GDEXT_ENVIRONMENT_VEGETATION_INSTANCE_ENABLED
	ClassDB::register_class<VegetationInstance>();
#endif
#endif
#ifdef GDEXT_ENVIRONMENT_SPIDER_ANIM_ENABLED
	ClassDB::register_class<Spider>();
	ClassDB::register_class<SpiderStage>();
	ClassDB::register_class<SpiderStageInstance>();
#endif
#ifdef GDEXT_ENVIRONMENT_STARFIELD_ENABLED
	ClassDB::register_class<Starfield2D>();
#endif

#ifdef TOOLS_ENABLED
#ifdef GDEXT_MEDIA_FLAC_ENABLED
	if (Engine::get_singleton()->is_editor_hint()) {
		Ref<ResourceImporterFLAC> flac_import;
		flac_import.instance();
		ResourceFormatImporter::get_singleton()->add_importer(flac_import);
	}
#endif
#ifdef GDEXT_MEDIA_GIFEXPORTER_ENABLED
	ClassDB::register_class<GifExporter>();
#endif
#endif // TOOLS_ENABLED

#ifdef GDEXT_MEDIA_FLAC_ENABLED
	ClassDB::register_class<AudioStreamFLAC>();
#endif
#ifdef GDEXT_MEDIA_SMACKVIDEO_ENABLED
	gdsmackvideo_init();
#endif
#ifdef GDEXT_MEDIA_FFMPEG_ENABLED
	gdffmpeg_init();
#endif
#ifdef TOOLS_ENABLED
#ifdef GDEXT_MEDIA_BMFIMPORTER_ENABLED
	Ref<BmfFontImporter> bmf_font = memnew(BmfFontImporter);
	ResourceFormatImporter::get_singleton()->add_importer(bmf_font);
#endif
#endif

#ifdef GDEXT_RUNTIMEPROFILER_ENABLED
	ClassDB::register_class<RuntimeProfiler>();
#endif

#ifdef GDEXT_BENCHMARK_ENABLED
	ClassDB::register_class<Benchmark>();
#endif

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

#ifdef GDEXT_SILENTWOLF_ENABLED
	Engine::get_singleton()->add_singleton(Engine::Singleton("SilentWolf", memnew(SilentWolf)));
	ClassDB::register_class<SilentWolfInstance>();
	ClassDB::register_virtual_class<SW_Auth>();
	ClassDB::register_virtual_class<SW_Scores>();
	ClassDB::register_virtual_class<SW_Player>();
	ClassDB::register_virtual_class<SW_Multiplayer>();
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
	ClassDB::register_class<QRCodeTexture>();
#endif // GDEXT_QRCODETEXTURE_ENABLED

#ifdef GDEXT_CCD_ENABLED
	ClassDB::register_class<CCDBox>();
	ClassDB::register_class<CCDSphere>();
	ClassDB::register_class<CCDCylinder>();
#endif // GDEXT_CCD_ENABLED

#ifdef GDEXT_POLYVECTOR_ENABLED
	ClassDB::register_class<PolyVector>();
	ClassDB::register_class<JSONVector>();

	resource_loader_jsonvector.instance();
	ResourceLoader::add_resource_format_loader(resource_loader_jsonvector);

#ifdef TOOLS_ENABLED
	if (Engine::get_singleton()->is_editor_hint()) {
		Ref<ResourceImporterSWF> swfdata;
		swfdata.instance();
		ResourceFormatImporter::get_singleton()->add_importer(swfdata);
	}
#endif
#endif

#ifdef GDEXT_THORVG_ENABLED
#ifdef TOOLS_ENABLED
	tvg::CanvasEngine tvgEngine = tvg::CanvasEngine::Sw;
	if (tvg::Initializer::init(tvgEngine, 1) != tvg::Result::Success) {
		return;
	}
	image_loader_thor_svg.instantiate();
	ImageLoader::add_image_format_loader(image_loader_thor_svg);
#endif
#endif

#ifdef GDEXT_MESHLOD_ENABLED
	ClassDB::register_class<MeshOptimize>();
	EditorPlugins::add_by_type<MeshOptimizePlugin>();
#endif

#ifdef GDEXT_MESHSLICER_ENABLED
	ClassDB::register_class<Slicer>();
	ClassDB::register_class<SlicedMesh>();
#endif

#ifdef GDEXT_SCENEMERGE_ENABLED
#ifdef TOOLS_ENABLED
	ClassDB::register_class<SceneMerge>();
	EditorPlugins::add_by_type<SceneMergePlugin>();
#endif
#endif

#ifdef GDEXT_TEXTUREPACKER_ENABLED
	ClassDB::register_class<TexturePacker>();
	ClassDB::register_class<TextureMerger>();
	ClassDB::register_class<PackerImageResource>();
	ClassDB::register_class<TextureLayerMerger>();
#ifdef TOOLS_ENABLED
	EditorPlugins::add_by_type<EditorPluginPackerImageResource>();
#endif
#endif

#ifdef GDEXT_SPACEMOUSE_ENABLED
	Engine::get_singleton()->add_singleton(Engine::Singleton("SpaceMouse", memnew(SpaceMouse)));
#endif

#ifdef TOOLS_ENABLED
	EditorNode::add_init_callback(editor_init_callback);
#endif
}

#define RemoveSingleton(S)                  \
	if (S *instance = S::get_singleton()) { \
		memdelete(instance);                \
	}

void unregister_gdextensions_types() {
	RemoveSingleton(ResCache);
	RemoveSingleton(Resources);
	if (thread_pool) {
		memdelete(thread_pool);
	}
#ifdef GDEXT_BEHAVIORTREE_ENABLED
	BTStringNames::free();
#endif
#ifdef GDEXT_BLITTER_ENABLED
	RemoveSingleton(BitBlit);
#endif
#ifdef GDEXT_CORE_ENABLED
	RemoveSingleton(Timer2);
	RemoveSingleton(Tween2);
	RemoveSingleton(InputStorage);
	RemoveSingleton(Tags);
	RemoveSingleton(Blitter);
#ifdef TOOLS_ENABLED
	RemoveSingleton(GodotErrorHandler);
#endif
#endif // GDEXT_CORE_ENABLED
#ifdef GDEXT_DEBUGDRAW_ENABLED
	RemoveSingleton(DebugDraw);
#endif
#ifdef GDEXT_POLYVECTOR_ENABLED
	ResourceLoader::remove_resource_format_loader(resource_loader_jsonvector);
	resource_loader_jsonvector.unref();
#endif
#ifdef GDEXT_THORVG_ENABLED
	if (!image_loader_thor_svg.is_null()) {
		ImageLoader::remove_image_format_loader(image_loader_thor_svg);
		image_loader_thor_svg.unref();
		tvg::Initializer::term(tvg::CanvasEngine::Sw);
	}
#endif
#ifdef GDEXT_SETTINGS_ENABLED
	RemoveSingleton(Settings);
#endif
#ifdef GDEXT_NAKAMA1_ENABLED
	RemoveSingleton(GdNakama1);
#endif
#ifdef GDEXT_PARSEPLATFORM_ENABLED
	RemoveSingleton(GdParseBackend);
#endif
#ifdef GDEXT_SILENTWOLF_ENABLED
	RemoveSingleton(SilentWolf);
#endif
#ifdef GDEXT_MULTIPEER_ENABLED
	RemoveSingleton(GdMultiPeer);
#endif
#ifdef GDEXT_HTTPSERVER_ENABLED
	RemoveSingleton(GdHttpServer);
#endif
#ifdef GDEXT_SPACEMOUSE_ENABLED
	RemoveSingleton(SpaceMouse);
#endif
#ifdef GDEXT_MEDIA_SMACKVIDEO_ENABLED
	gdsmackvideo_terminate();
#endif
#ifdef GDEXT_MEDIA_FFMPEG_ENABLED
	gdffmpeg_terminate();
#endif
#ifdef GDEXT_MEDIA_FFMPEG_ENABLED
	gdffmpeg_init();
#endif
#ifdef GDEXT_IAP_ENABLED
#if defined(OSX_ENABLED) || defined(UWP_ENABLED) || defined(IPHONE_ENABLED)
	GdInAppStore::release_store();
#endif
#endif // GDEXT_IAP_ENABLED
}
