
#include "core/reference.h"

typedef enum {
	SIMD_SSE3,
	SIMD_SSE4_1,
	SIMD_SSE4_2,
	SIMD_AVX,
	SIMD_AVX2,
	SIMD_NEON
} SIMD_INTRINSIC;

struct CpuInfo;

class CpuFeatures : public Reference {
	GDCLASS(CpuFeatures, Reference);

	Ref<CpuInfo> cpu_info;

#if defined(ANDROID)
	bool init_cpu_info(CpuInfo* out_cpu_info, JNIEnv* pJavaEnv);
#else
	bool init_cpu_info(CpuInfo* out_cpu_info);
#endif

protected:
	static void _bind_methods();

public:
	CpuFeatures();
};
