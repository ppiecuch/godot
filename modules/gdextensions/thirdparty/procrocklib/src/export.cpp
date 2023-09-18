#include <procrocklib/export.h>

#include <igl/writeOBJ.h>

#include <cstdio>

namespace procrock {
static void dumpimage(const std::string path, const int width, const int height, const int depth, const uint8_t *buffer) {
  FILE* out = fopen((path + ".tga").c_str(), "wb");

  uint8_t *data = (uint8_t*)buffer;
  uint8_t bpp = depth*8;
  if(depth == 1) { // upscale from 8 to 16bit
    data = (uint8_t*)malloc(width * height * 2);
    for(int i = 0; i < width*height*depth; i++) {
      uint16_t tmp = 0;
      #define BIT(index) (1 << (index))
      #define FIVE_BITS (BIT(0)|BIT(1)|BIT(2)|BIT(3)|BIT(4))
      tmp  =  (buffer[i] >> 3) & FIVE_BITS; // r
      tmp |= ((buffer[i] >> 3) & FIVE_BITS) << 5; // g
      tmp |= ((buffer[i] >> 3) & FIVE_BITS) << 10; //b
      if (buffer[i] > 127) tmp |= BIT(15); // a
      #undef FIVE_BITS
      #undef BIT
      data[i*2+0] = (uint8_t) (tmp & 0x00FF);
      data[i*2+1] = (uint8_t)((tmp & 0xFF00) >> 8);
    }
    bpp = 16;
  }

  uint8_t tga_header[18] = { 0 };
  tga_header[2] = 2; // Data code type -- 2 - uncompressed RGB image.
  tga_header[12] = width & 0xFF; // Image width - low byte
  tga_header[13] = (width >> 8) & 0xFF; // Image width - high byte
  tga_header[14] = height & 0xFF; // Image height - low byte
  tga_header[15] = (height >> 8) & 0xFF; // Image height - high byte
  tga_header[16] = bpp; // Color bit depth - 16,24,32
  if(depth == 4)
    tga_header[17] = 4; // bottom left image (0x00) + 8 bit alpha (0x4)
  fwrite(tga_header, sizeof(uint8_t), 18, out);

  // save flipped data:
  const size_t lsize = width * depth;
  const uint8_t *data_end = data + width*height*depth - lsize;
  for(int y = 0; y < height; y++) {
    fwrite(data_end - y*lsize, sizeof(uint8_t), lsize, out);
  }

  fclose(out);

  if(data != buffer)
    free( data );
}

void exportMesh(Mesh& mesh, const std::string filepath, bool albedo, bool normals, bool roughness, bool metal, bool displace, bool ambientOcc) {
  igl::writeOBJ(filepath, mesh.vertices, mesh.faces, mesh.normals, mesh.faces, mesh.uvs, mesh.faces);

  std::string base = filepath;

  // Remove extension if present.
  const size_t period_idx = base.rfind('.');
  if (std::string::npos != period_idx) {
    base.erase(period_idx);
  }

  base = base + "-tex-";

  if (albedo) {
    std::string albedoFile = base + "albedo";
    dumpimage(albedoFile.c_str(), mesh.textures.width, mesh.textures.height, 3, mesh.textures.albedoData.data());
  }

  if (normals) {
    std::string normalFile = base + "normal";
    dumpimage(normalFile.c_str(), mesh.textures.width, mesh.textures.height, 3, mesh.textures.normalData.data());
  }

  if (metal) {
    std::string metalFile = base + "metal";
    dumpimage(metalFile.c_str(), mesh.textures.width, mesh.textures.height, 1, mesh.textures.metalData.data());
  }

  if (roughness) {
    std::string roughnessFile = base + "roughness";
    dumpimage(roughnessFile.c_str(), mesh.textures.width, mesh.textures.height, 1, mesh.textures.roughnessData.data());
  }

  if (ambientOcc) {
    std::string ambientOccFile = base + "ambientOcc";
    dumpimage(ambientOccFile.c_str(), mesh.textures.width, mesh.textures.height, 1, mesh.textures.ambientOccData.data());
  }

  if (displace) {
    std::vector<unsigned char> displacementExport;
    displacementExport.resize(mesh.textures.displacementData.size());
    for (int i = 0; i < mesh.textures.displacementData.size(); i++) {
      displacementExport[i] = mesh.textures.displacementData[i] * 255;
    }
    std::string displacementFile = base + "displacement";
    dumpimage(displacementFile.c_str(), mesh.textures.width, mesh.textures.height, 1, displacementExport.data());
  }
}
}  // namespace procrock
