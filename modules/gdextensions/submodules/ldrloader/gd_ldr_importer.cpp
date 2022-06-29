/*************************************************************************/
/*  gd_ldr_importer.cpp                                                  */
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

#include "gd_ldr_importer.h"

#include "libldr/color.h"
#include "libldr/elements.h"
#include "libldr/geometry_exporter.h"
#include "libldr/lutils.h"
#include "libldr/metrics.h"
#include "libldr/model.h"
#include "libldr/part_library.h"
#include "libldr/reader.h"

#include <memory>

// -------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool LDRImporter::CanRead(const std::string &pFile, IOSystem *pIOHandler,
		bool checkSig) const {
	const std::string extension = GetExtension(pFile);
	if (extension == "ldr" || extension == "mpd" || extension == "dat") {
		return true;
	}
	if (!extension.length() || checkSig) {
		// no extension given, or we're called a second time because no
		// suitable loader was found yet. This means, we're trying to open
		// the file and look for and hints to identify the file format.
		// #Assimp::BaseImporter provides some utilities:
		//
		// #Assimp::BaseImporter::SearchFileHeaderForToken - for text files.
		// It reads the first lines of the file and does a substring check
		// against a given list of 'magic' strings.
		//
		// #Assimp::BaseImporter::CheckMagicToken - for binary files. It goes
		// to a particular offset in the file and and compares the next words
		// against a given list of 'magic' tokens.
		// These checks MUST be done (even if !checkSig) if the file extension
		// is not exclusive to your format. For example, .xml is very common
		// and (co)used by many formats.
	}
	return false;
}
// ------------------------------------------------------------------------------------------------
// Setup configuration properties
void LDRImporter::SetupProperties(const Importer * /*pImp*/) {
	// nothing to be done for the moment
}
// ------------------------------------------------------------------------------------------------
// Loader registry entry
const aiImporterDesc *LDRImporter::GetInfo() const {
	return &desc_ldraw;
}
// -------------------------------------------------------------------------------
// Get list of file extensions handled by this loader
void LDRImporter::GetExtensionList(std::set<std::string> &extensions) {
	extensions.insert("ldr");
	extensions.insert("mpd");
	extensions.insert("dat");
}
// -------------------------------------------------------------------------------
inline static std::string Filename(const std::string path, const char *sep = "/") {
	size_t slash = path.find_last_of(sep);
	if (slash == std::string::npos)
		return path;
	return path.substr(slash + 1);
}
void LDRImporter::InternReadFile(const std::string &pFile,
		aiScene *pScene, IOSystem *pIOHandler) {
	const bool DISPLAY_COND_LINES = true;

	if (m_library == 0) {
		try {
			m_library = new ldraw::part_library();
		} catch (const ldraw::exception &e) {
			throw DeadlyImportError(std::string("LDR: Cannot initialize part library - ").append(e.what()));
		}

		ldraw::color::init();

		m_library->set_unlink_policy(ldraw::part_library::parts);
		m_reader = new ldraw::reader();
		m_reader->enable_global_cache();
	}

	StreamReaderLE stream(pIOHandler->Open(pFile, "rb"));

	// Your task: fill pScene
	// Throw a ImportErrorException with a meaningful (!) error message if
	// something goes wrong.

	istreamreader<StreamReaderLE> istreamr(&stream);

	// load the model
	ldraw::model_multipart *m;
	try {
		m = m_reader->load_from_stream(istreamr, Filename(pFile));
	} catch (const ldraw::exception &e) {
		throw DeadlyImportError(e.what());
	}

#ifndef LDR_ARCHIVE_SUPPORT
#error "Embedded archive support is required for this module to build."
#endif

	// optimize loading to avoid trashing gzip file:
	std::vector<ldcatalog::handle_t> load_list;
	// linking list:
	std::function<void(const std::vector<ldcatalog::handle_t> &, std::vector<ldcatalog::handle_t> &)> check_deps;
	check_deps = [&](const std::vector<ldcatalog::handle_t> &hndls, std::vector<ldcatalog::handle_t> &lst) {
		for (auto &h : hndls)
			if (std::find(lst.begin(), lst.end(), h) == lst.end()) {
				lst.push_back(h);
				check_deps(m_reader->get_archive_deps(h), lst);
			}
	};
	auto opt = [=](ldraw::model *m, std::vector<ldcatalog::handle_t> &lst) {
		for (int i = 0; i < m->size(); ++i) {
			if (m->at(i)->get_type() == ldraw::type_ref) {
				ldraw::element_ref *r = CAST_AS_REF(m->at(i));
				const std::string fn = ldraw::utils::translate_string(r->filename());
				if (m_reader->get_archive_exists(fn)) {
					const ldcatalog::handle_t h = m_reader->get_archive_handle(fn);
					if (std::find(lst.begin(), lst.end(), h) == lst.end()) {
						lst.push_back(h);
						check_deps(m_reader->get_archive_deps(fn), lst);
					}
				}
			}
		}
	};

	opt(m->main_model(), load_list);
	for (auto &sm : m->submodel_list())
		opt(sm.second, load_list);
	// sort according to offset in archive:
	auto _compare = [=](const ldcatalog::handle_t &left, const ldcatalog::handle_t &right) {
		return m_reader->get_archive_offs(left) < m_reader->get_archive_offs(right);
	};
	std::sort(load_list.begin(), load_list.end(), _compare);
#ifdef DEBUG
	std::cout << "Loading order:" << std::endl;
	for (auto &h : load_list)
		std::cout << " model " << m_reader->get_archive_model(h) << " (" << h << "), offset " << m_reader->get_archive_offs(h) << std::endl;
#endif
	// preload objects:
	{
		const clock_t first = ::clock();

		for (auto &h : load_list)
			try {
				ldraw::item_refcount *r = m_reader->load_from_archive(h);
			} catch (const ldraw::exception &e) {
				ASSIMP_LOG_INFO_F("Failed to preload model id: %u", h);
			}

		const clock_t second = ::clock();
		const double seconds = static_cast<double>(second - first) / CLOCKS_PER_SEC;
		printf("Preloading LDR model ...              OK \n   took approx. %.5f seconds\n"
			   "\n",
				seconds);
	}

	ASSIMP_LOG_INFO("LDR: Linking model");
	{
		const clock_t first = ::clock();

		m_library->link(m);

		const clock_t second = ::clock();
		const double seconds = static_cast<double>(second - first) / CLOCKS_PER_SEC;
		printf("Linking LDR model ...                OK \n   link took approx. %.5f seconds\n"
			   "\n",
				seconds);
	}

	using namespace lexporter;

	geometry_exporter::vbuffer_params params;

	ASSIMP_LOG_INFO("LDR: Generating geometry");
	geometry_exporter xp(m->main_model(), &params);
	xp.update(true); // collapse

	std::stack<ldraw::color> colorstack;
	colorstack.push(ldraw::color(7));

	pScene->mRootNode = new aiNode();
	pScene->mRootNode->mName.Set("<LDRRoot>");

	if (int(xp.count(geometry_exporter::buffer_type::type_lines) > 0) + int(xp.count(geometry_exporter::buffer_type::type_triangles) > 0) + int(xp.count(geometry_exporter::buffer_type::type_quads) > 0) == 0) // lines-triangles-quads
		throw DeadlyImportError("LDR: geometry is empty. There are no meshes defined");

	auto copyVertices = [](aiMesh *mesh, const float *verts, const size_t count) {
		aiVector3D *v = mesh->mVertices = new aiVector3D[count];
		mesh->mNumVertices = count;
#ifdef ASSIMP_DOUBLE_PRECISION
		for (unsigned int t = 0; t < mesh->mNumVertices; t += 3, ++v) {
			v->x = verts[t];
			v->y = verts[t + 1];
			v->z = verts[t + 2];
		}
#else
		std::memcpy(mesh->mVertices, verts, 3 * count * sizeof(float));
#endif
	};

	std::vector<unsigned int> allMaterials;
	std::vector<aiMesh *> allMeshes;

	auto getMaterialIndex = [=](std::vector<unsigned int> &mats, const unsigned int color) {
		auto pos = std::find(mats.begin(), mats.end(), color) - mats.begin();
		if (pos >= mats.size()) {
			mats.push_back(color);
			return size_t(mats.size() - 1);
		}

		return size_t(pos);
	};

	/* lines */
	if (xp.count(geometry_exporter::buffer_type::type_lines) > 0) {
		const unsigned int count = xp.count(geometry_exporter::buffer_type::type_lines);
		const float *verts = xp.get_vertex_array(geometry_exporter::buffer_type::type_lines);
		const unsigned int *colors = xp.get_color_index(geometry_exporter::buffer_type::type_lines);

		const int numFaces = count / 2;

		std::map<unsigned int, int> unique_colors;
		for (int c = 0; c < numFaces; ++c)
			unique_colors[colors[c]]++;

		// build mesh for each material/color:
		int cnt = 1;
		for (auto &col : unique_colors) {
			aiMesh *mesh = new aiMesh();
			allMeshes.push_back(mesh);
			mesh->mName = "ldr.lines." + std::to_string(cnt);
			mesh->mMaterialIndex = getMaterialIndex(allMaterials, col.first);
			mesh->mPrimitiveTypes = aiPrimitiveType_LINE;
			mesh->mNumFaces = col.second;
			mesh->mNumVertices = mesh->mNumFaces * 2;
			aiFace *faces = mesh->mFaces = new aiFace[mesh->mNumFaces];
			aiVector3D *vertices = mesh->mVertices = new aiVector3D[mesh->mNumVertices];
			unsigned int index = 0;
			for (unsigned int t = 0; t < numFaces; ++t) {
				if (colors[t] == col.first) {
					aiFace *face = &faces[index];
					aiVector3D *line = &vertices[index * 2];
					face->mNumIndices = 2;
					face->mIndices = new unsigned int[face->mNumIndices]{ index * 2, index * 2 + 1 };
					for (unsigned int v = 0; v < 2; ++v) {
						line[v].x = verts[6 * t + v * 3];
						line[v].y = verts[6 * t + v * 3 + 1];
						line[v].z = verts[6 * t + v * 3 + 2];
					}
					++index;
				}
			}
			++cnt;
		}
	}

	/* cond. lines */
	if (DISPLAY_COND_LINES && xp.count(geometry_exporter::buffer_type::type_condlines) > 0) {
		const unsigned int count = xp.count(geometry_exporter::buffer_type::type_condlines);
		const float *verts = xp.get_vertex_array(geometry_exporter::buffer_type::type_condlines);
		const unsigned int *colors = xp.get_color_index(geometry_exporter::buffer_type::type_condlines);

		const int numFaces = count / 2;

		std::map<unsigned int, int> unique_colors;
		for (int c = 0; c < numFaces; ++c)
			unique_colors[colors[c]]++;

		// build mesh for each material/color:
		int cnt = 1;
		for (auto &col : unique_colors) {
			aiMesh *mesh = new aiMesh();
			allMeshes.push_back(mesh);
			mesh->mName = "ldr.condlines." + std::to_string(cnt);
			mesh->mMaterialIndex = getMaterialIndex(allMaterials, col.first);
			mesh->mPrimitiveTypes = aiPrimitiveType_LINE;
			mesh->mNumFaces = col.second;
			mesh->mNumVertices = mesh->mNumFaces * 2;
			aiFace *faces = mesh->mFaces = new aiFace[mesh->mNumFaces];
			aiVector3D *vertices = mesh->mVertices = new aiVector3D[mesh->mNumVertices];
			unsigned int index = 0;
			for (unsigned int t = 0; t < numFaces; ++t) {
				if (colors[t] == col.first) {
					aiFace *face = &faces[index];
					aiVector3D *line = &vertices[index * 2];
					face->mNumIndices = 2;
					face->mIndices = new unsigned int[face->mNumIndices]{ index * 2, index * 2 + 1 };
					for (unsigned int v = 0; v < 2; ++v) {
						line[v].x = verts[6 * t + v * 3];
						line[v].y = verts[6 * t + v * 3 + 1];
						line[v].z = verts[6 * t + v * 3 + 2];
					}
					++index;
				}
			}
			++cnt;
		}
	}

	/* triangles */
	if (xp.count(geometry_exporter::buffer_type::type_triangles) > 0) {
		const unsigned int count = xp.count(geometry_exporter::buffer_type::type_triangles);
		const float *verts = xp.get_vertex_array(geometry_exporter::buffer_type::type_triangles);
		const unsigned int *colors = xp.get_color_index(geometry_exporter::buffer_type::type_triangles);

		const int numFaces = count / 3;

		std::map<unsigned int, int> unique_colors;
		for (int c = 0; c < numFaces; ++c)
			unique_colors[colors[c]]++;

		// build mesh for each material/color:
		int cnt = 1;
		for (auto &col : unique_colors) {
			aiMesh *mesh = new aiMesh();
			allMeshes.push_back(mesh);
			mesh->mName = "ldr.triangles." + std::to_string(cnt);
			mesh->mMaterialIndex = getMaterialIndex(allMaterials, col.first);
			mesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;
			mesh->mNumFaces = col.second;
			mesh->mNumVertices = mesh->mNumFaces * 3;
			aiFace *faces = mesh->mFaces = new aiFace[mesh->mNumFaces];
			aiVector3D *vertices = mesh->mVertices = new aiVector3D[mesh->mNumVertices];
			unsigned int index = 0;
			for (unsigned int t = 0; t < numFaces; ++t) {
				if (colors[t] == col.first) {
					aiFace *face = &faces[index];
					aiVector3D *trig = &vertices[index * 3];
					face->mNumIndices = 3;
					face->mIndices = new unsigned int[face->mNumIndices]{ index * 3, index * 3 + 1, index * 3 + 2 };
					for (unsigned int v = 0; v < 3; ++v) {
						trig[v].x = verts[9 * t + v * 3];
						trig[v].y = verts[9 * t + v * 3 + 1];
						trig[v].z = verts[9 * t + v * 3 + 2];
					}
					++index;
				}
			}
			++cnt;
		}
	}

	/* quads */
	if (xp.count(geometry_exporter::buffer_type::type_quads) > 0) {
		const unsigned int count = xp.count(geometry_exporter::buffer_type::type_quads);
		const float *verts = xp.get_vertex_array(geometry_exporter::buffer_type::type_quads);
		const unsigned int *colors = xp.get_color_index(geometry_exporter::buffer_type::type_quads);

		const int numFaces = count / 4;

		std::map<unsigned int, int> unique_colors;
		for (int c = 0; c < numFaces; ++c)
			unique_colors[colors[c]]++;

		// build mesh for each material/color:
		int cnt = 1;
		for (auto &col : unique_colors) {
			aiMesh *mesh = new aiMesh();
			allMeshes.push_back(mesh);
			mesh->mName = "ldr.quads." + std::to_string(cnt);
			mesh->mMaterialIndex = getMaterialIndex(allMaterials, col.first);
			mesh->mPrimitiveTypes = aiPrimitiveType_POLYGON;
			mesh->mNumFaces = col.second;
			mesh->mNumVertices = mesh->mNumFaces * 4;
			aiFace *faces = mesh->mFaces = new aiFace[mesh->mNumFaces];
			aiVector3D *vertices = mesh->mVertices = new aiVector3D[mesh->mNumVertices];
			unsigned int index = 0;
			for (unsigned int t = 0; t < numFaces; ++t) {
				if (colors[t] == col.first) {
					aiFace *face = &faces[index];
					aiVector3D *quad = &vertices[index * 4];
					face->mNumIndices = 4;
					face->mIndices = new unsigned int[face->mNumIndices]{ index * 4, index * 4 + 1, index * 4 + 2, index * 4 + 3 };
					for (unsigned int v = 0; v < 4; ++v) {
						quad[v].x = verts[12 * t + v * 3];
						quad[v].y = verts[12 * t + v * 3 + 1];
						quad[v].z = verts[12 * t + v * 3 + 2];
					}
					++index;
				}
			}
			++cnt;
		}
	}

	pScene->mNumMeshes = pScene->mRootNode->mNumMeshes = allMeshes.size();
	pScene->mRootNode->mMeshes = new unsigned int[pScene->mRootNode->mNumMeshes];
	for (int i = 0; i < pScene->mNumMeshes; i++)
		pScene->mRootNode->mMeshes[i] = i;
	pScene->mRootNode->mMeshes[0] = 0;
	pScene->mMeshes = new aiMesh *[pScene->mNumMeshes];
	memcpy(pScene->mMeshes, &allMeshes[0], pScene->mNumMeshes * sizeof(aiMesh *));

	pScene->mNumMaterials = allMaterials.size();
	pScene->mMaterials = new aiMaterial *[pScene->mNumMaterials];
#ifdef DEBUG
	std::cout << "Building " << allMaterials.size() << " materials." << std::endl;
#endif
	int mindex = 0;
	for (auto &m : allMaterials) {
		const unsigned char *ce = (const unsigned char *)&m;

		aiMaterial *pcMat = new aiMaterial();
		aiColor4D clr((ai_real(ce[0] / 255.)), (ai_real(ce[1] / 255.)), (ai_real(ce[2] / 255.)), (ai_real(ce[3] / 255.)));
		pcMat->AddProperty(&clr, 1, AI_MATKEY_COLOR_DIFFUSE);
		const int twosided = 1;
		pcMat->AddProperty(&twosided, 1, AI_MATKEY_TWOSIDED);

		pScene->mMaterials[mindex] = pcMat;

		++mindex;
	}

	printf("Importing LDR model ...              OK\n");
}
