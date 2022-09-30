#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void save_dat(const char *app, const char *name, const float verts[], uint32_t verts_num, const uint16_t indexes[], uint32_t indexes_num) {
	FILE *file = fopen(name, "w");
	if (file == nullptr) {
		printf("%s: Cannot write '%s'.\n", app, name);
		exit(1);
	}
	fwrite(&verts_num, sizeof(uint32_t), 1, file);
	uint32_t verts_size = verts_num * sizeof(float) * 8;
	printf("%s: %d vertexes, %d bytes of data.\n", app, verts_num, verts_size);
	fwrite(verts, verts_size, 1, file);
	fwrite(&indexes_num, sizeof(uint32_t), 1, file);
	uint32_t indexes_size = indexes_num * sizeof(uint16_t);
	printf("%s: %d indexes, %d bytes of data.\n", app, indexes_num, indexes_size);
	indexes = (uint16_t*)malloc(indexes_size);
	fwrite(indexes, indexes_size, 1, file);
	fclose(file);
}

int main(int argc, char *argv[]) {
	// save_dat(argv[0], "cube.dat", cube_verts, sizeof(cube_verts)/sizeof(float), cube_indices, sizeof(cube_indices)/sizeof(uint16_t));
}
