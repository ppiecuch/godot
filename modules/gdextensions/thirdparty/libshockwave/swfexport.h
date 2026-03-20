#ifndef LIBSHOCKWAVE_SWF_EXPORT_H
#define LIBSHOCKWAVE_SWF_EXPORT_H

#include <cstdio>
#include <cstring>
#include <string>

#include "swftypedefs.h"

namespace SWF
{
	class SVGExporter
	{
	public:
		static std::string export_shape(const Dictionary &dict, uint16_t character_id)
		{
			std::string svg;
			CharacterDict::const_iterator it = dict.CharacterList.find(character_id);
			if (it == dict.CharacterList.end()) return svg;

			const Character &ch = it->second;
			const Rect &bounds = ch.bounds;

			char viewbox[256];
			snprintf(viewbox, sizeof(viewbox),
				"<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"%.2f %.2f %.2f %.2f\">\n",
				bounds.xmin, bounds.ymin,
				bounds.xmax - bounds.xmin, bounds.ymax - bounds.ymin);
			svg += viewbox;

			FillStyleMap::const_iterator fill_it = dict.FillStyles.find(character_id);
			LineStyleMap::const_iterator line_it = dict.LineStyles.find(character_id);

			for (size_t s = 0; s < ch.shapes.size(); s++) {
				const Shape &shape = ch.shapes[s];
				if (shape.vertices.size() < 2) continue;

				std::string path_d;
				for (size_t v = 0; v < shape.vertices.size(); v++) {
					const Vertex &vert = shape.vertices[v];
					if (v == 0) {
						char buf[64];
						snprintf(buf, sizeof(buf), "M%.2f %.2f", vert.anchor.x, vert.anchor.y);
						path_d += buf;
					} else {
						if (vert.anchor.x == vert.control.x && vert.anchor.y == vert.control.y) {
							// Straight line
							char buf[64];
							snprintf(buf, sizeof(buf), " L%.2f %.2f", vert.anchor.x, vert.anchor.y);
							path_d += buf;
						} else {
							// Quadratic bezier
							char buf[128];
							snprintf(buf, sizeof(buf), " Q%.2f %.2f %.2f %.2f",
								vert.control.x, vert.control.y,
								vert.anchor.x, vert.anchor.y);
							path_d += buf;
						}
					}
				}
				if (shape.closed) path_d += " Z";

				svg += "  <path d=\"";
				svg += path_d;
				svg += "\"";

				// Fill
				if (shape.fill1 > 0 && fill_it != dict.FillStyles.end()) {
					uint16_t fill_idx = shape.fill1 - 1;
					if (fill_idx < fill_it->second.size()) {
						const FillStyle &fs = fill_it->second[fill_idx];
						if (fs.StyleType == FillStyle::Type::SOLID) {
							char fill_str[64];
							snprintf(fill_str, sizeof(fill_str),
								" fill=\"rgb(%d,%d,%d)\"", fs.Color.r, fs.Color.g, fs.Color.b);
							svg += fill_str;
							if (fs.Color.a < 255) {
								char alpha_str[32];
								snprintf(alpha_str, sizeof(alpha_str),
									" fill-opacity=\"%.2f\"", fs.Color.a / 255.0f);
								svg += alpha_str;
							}
						}
					}
				} else {
					svg += " fill=\"none\"";
				}

				// Stroke
				if (shape.stroke > 0 && line_it != dict.LineStyles.end()) {
					uint16_t line_idx = shape.stroke - 1;
					if (line_idx < line_it->second.size()) {
						const LineStyle &ls = line_it->second[line_idx];
						char stroke_str[128];
						snprintf(stroke_str, sizeof(stroke_str),
							" stroke=\"rgb(%d,%d,%d)\" stroke-width=\"%.2f\"",
							ls.Color.r, ls.Color.g, ls.Color.b, ls.Width);
						svg += stroke_str;
					}
				}

				svg += "/>\n";
			}

			svg += "</svg>";
			return svg;
		}
	};

	class JSONExporter
	{
	public:
		static std::string export_shape(const Dictionary &dict, uint16_t character_id)
		{
			std::string json;
			CharacterDict::const_iterator it = dict.CharacterList.find(character_id);
			if (it == dict.CharacterList.end()) return "{}";

			const Character &ch = it->second;
			const Rect &bounds = ch.bounds;

			json += "{\n";

			// Bounds
			char bounds_str[256];
			snprintf(bounds_str, sizeof(bounds_str),
				"  \"bounds\": {\"xmin\": %.2f, \"ymin\": %.2f, \"xmax\": %.2f, \"ymax\": %.2f},\n",
				bounds.xmin, bounds.ymin, bounds.xmax, bounds.ymax);
			json += bounds_str;

			json += "  \"shapes\": [\n";

			FillStyleMap::const_iterator fill_it = dict.FillStyles.find(character_id);
			LineStyleMap::const_iterator line_it = dict.LineStyles.find(character_id);

			for (size_t s = 0; s < ch.shapes.size(); s++) {
				const Shape &shape = ch.shapes[s];
				if (s > 0) json += ",\n";

				json += "    {\n";
				json += "      \"closed\": ";
				json += shape.closed ? "true" : "false";
				json += ",\n";

				// Fill info
				if (shape.fill1 > 0 && fill_it != dict.FillStyles.end()) {
					uint16_t fill_idx = shape.fill1 - 1;
					if (fill_idx < fill_it->second.size()) {
						const FillStyle &fs = fill_it->second[fill_idx];
						char fill_str[128];
						snprintf(fill_str, sizeof(fill_str),
							"      \"fill\": {\"r\": %d, \"g\": %d, \"b\": %d, \"a\": %d},\n",
							fs.Color.r, fs.Color.g, fs.Color.b, fs.Color.a);
						json += fill_str;
					}
				}

				// Stroke info
				if (shape.stroke > 0 && line_it != dict.LineStyles.end()) {
					uint16_t line_idx = shape.stroke - 1;
					if (line_idx < line_it->second.size()) {
						const LineStyle &ls = line_it->second[line_idx];
						char stroke_str[128];
						snprintf(stroke_str, sizeof(stroke_str),
							"      \"stroke\": {\"r\": %d, \"g\": %d, \"b\": %d, \"width\": %.2f},\n",
							ls.Color.r, ls.Color.g, ls.Color.b, ls.Width);
						json += stroke_str;
					}
				}

				json += "      \"commands\": [\n";

				for (size_t v = 0; v < shape.vertices.size(); v++) {
					const Vertex &vert = shape.vertices[v];
					if (v > 0) json += ",\n";

					if (v == 0) {
						char cmd[128];
						snprintf(cmd, sizeof(cmd),
							"        {\"type\": \"M\", \"x\": %.2f, \"y\": %.2f}",
							vert.anchor.x, vert.anchor.y);
						json += cmd;
					} else if (vert.anchor.x == vert.control.x && vert.anchor.y == vert.control.y) {
						char cmd[128];
						snprintf(cmd, sizeof(cmd),
							"        {\"type\": \"L\", \"x\": %.2f, \"y\": %.2f}",
							vert.anchor.x, vert.anchor.y);
						json += cmd;
					} else {
						char cmd[256];
						snprintf(cmd, sizeof(cmd),
							"        {\"type\": \"Q\", \"cx\": %.2f, \"cy\": %.2f, \"x\": %.2f, \"y\": %.2f}",
							vert.control.x, vert.control.y,
							vert.anchor.x, vert.anchor.y);
						json += cmd;
					}
				}

				json += "\n      ]\n    }";
			}

			json += "\n  ]\n}";
			return json;
		}
	};

}

#endif // LIBSHOCKWAVE_SWF_EXPORT_H
