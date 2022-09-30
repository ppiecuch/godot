#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char *argv[]) {

	if (argc < 2) {
		printf("%s: Filename required.\n", argv[0]);
		exit(1);
	}

	FILE *file = fopen(argv[1], "r");

	if (file == nullptr) {
		printf("%s: Cannot open '%s'.\n", argv[0], argv[1]);
		exit(2);
	}

	uint32_t verts_num = -1, indexes_num = -1;
	float *verts = nullptr;
	uint16_t *indexes = nullptr;

	fread(&verts_num, sizeof(uint32_t), 1, file);
	uint32_t verts_size = verts_num * sizeof(float) * 8;
	printf("%s: %d vertexes, %d bytes of data.\n", argv[0], verts_num, verts_size);
	verts = (float*)malloc(verts_size);
	fread(verts, verts_size, 1, file);
	fread(&indexes_num, sizeof(uint32_t), 1, file);
	uint32_t indexes_size = indexes_num * sizeof(uint16_t);
	printf("%s: %d indexes, %d bytes of data.\n", argv[0], indexes_num, indexes_size);
	indexes = (uint16_t*)malloc(indexes_size);
	fread(indexes, indexes_size, 1, file);
	fclose(file);
}
