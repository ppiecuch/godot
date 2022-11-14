#include "gd_cpu_features.h"

#include "cpu_features/cpu_features_types.h"

#include "cpu_features/cpuinfo_x86.h"
#include "cpu_features/cpuinfo_aarch64.h"

#define MAXCPUNAME 49

static char* trim_string(char* in_string);

struct CpuInfo : public Reference {
	char name[512];
	SIMD_INTRINSIC simd;

	X86Features features_x86;
	X86Microarchitecture architecture_x86;

	Aarch64Features features_aarch64;
};

#if defined(ANDROID)
bool CpuFeatures::init_cpu_info(CpuInfo *out_cpu_info, JNIEnv* pJavaEnv)
#else
bool CpuFeatures::init_cpu_info(CpuInfo *out_cpu_info)
#endif
{
	bool result = false;
	out_cpu_info->name[0] = '\0';

#if defined(CPU_FEATURES_ARCH_X86) && !defined(TARGET_IOS_SIMULATOR)
	X86Info info = {};

	const char* simd_name = "Unknown";

	//get cpu data
	result = GetX86Info(&info);

	if (result) {
		//orbis and prospero do not provide cpu names
#if defined(ORBIS)
		snprintf(info.name, sizeof(info.name), "Orbis");
#elif defined(PROSPERO)
		snprintf(info.name, sizeof(info.name), "Prospero");
#else
		char cpu_name[MAXCPUNAME] = "";
		FillX86BrandString(cpu_name);
		char* trimmed_name = trim_string(cpu_name);
		snprintf(info.name, sizeof(info.name), "%s", trimmed_name);
#endif

		//detect simd
		if (info.features.avx2) {
			out_cpu_info->simd = SIMD_AVX2;
			simd_name = "SIMD: AVX2";
		} else if (info.features.avx) {
			out_cpu_info->simd = SIMD_AVX;
			simd_name = "SIMD: AVX";
		} else if (info.features.sse4_2) {
			out_cpu_info->simd = SIMD_SSE4_2;
			simd_name = "SIMD: SSE4.2";
		} else if (info.features.sse4_1) {
			out_cpu_info->simd = SIMD_SSE4_1;
			simd_name = "SIMD: SSE4.1";
		}
	}

	out_cpu_info->features_x86 = info.features;
	out_cpu_info->architecture_x86 = GetX86Microarchitecture(&info);

	snprintf(out_cpu_info->name, sizeof(out_cpu_info->name), "%s \t\t\t\t\t %s", info.name, simd_name);
#endif

#if defined(CPU_FEATURES_ARCH_AARCH64) || defined(TARGET_IOS_SIMULATOR)
	Aarch64Info info = {};

	const char* simd_name = "SIMD: NEON";
	out_cpu_info->simd = SIMD_NEON;

	//ARM64 supported platforms by cpu_features
#if defined(ANDROID) || defined(__LINUX__) || defined(TARGET_APPLE_ARM64)
	result = GetAarch64Info(&info);
#endif

#if defined(ANDROID)
	jclass classBuild = pJavaEnv->FindClass("android/os/Build");

	jfieldID field;
	jstring jHardwareString, jBrandString, jBoardString, jModelString;
	const char* hardwareString, *brandStrirng, *boardString, *modelString;

	field = pJavaEnv->GetStaticFieldID(classBuild, "HARDWARE", "Ljava/lang/String;");
	jHardwareString = (jstring)pJavaEnv->GetStaticObjectField(classBuild, field);
	hardwareString = pJavaEnv->GetStringUTFChars(jHardwareString, 0);

	field = pJavaEnv->GetStaticFieldID(classBuild, "BRAND", "Ljava/lang/String;");
	jBrandString = (jstring)pJavaEnv->GetStaticObjectField(classBuild, field);
	brandStrirng = pJavaEnv->GetStringUTFChars(jBrandString, 0);

	field = pJavaEnv->GetStaticFieldID(classBuild, "BOARD", "Ljava/lang/String;");
	jBoardString = (jstring)pJavaEnv->GetStaticObjectField(classBuild, field);
	boardString = pJavaEnv->GetStringUTFChars(jBoardString, 0);

	field = pJavaEnv->GetStaticFieldID(classBuild, "MODEL", "Ljava/lang/String;");
	jModelString = (jstring)pJavaEnv->GetStaticObjectField(classBuild, field);
	modelString = pJavaEnv->GetStringUTFChars(jModelString, 0);

	snprintf(info.name, sizeof(info.name), "%s %s %s %s   ", hardwareString, brandStrirng, boardString, modelString);

	pJavaEnv->GetStringUTFChars(jHardwareString, 0);
	pJavaEnv->GetStringUTFChars(jBrandString, 0);
	pJavaEnv->GetStringUTFChars(jBoardString, 0);
	pJavaEnv->GetStringUTFChars(jModelString, 0);

#endif

	out_cpu_info->features_aarch64 = info.features;

	snprintf(out_cpu_info->name, sizeof(out_cpu_info->name), "%s\t\t\t\t\t %s", info.name, simd_name);
#endif

	return result;
}

static char* trim_string(char* in_string) {
	//trim end
	char* trimmed_string = in_string + (MAXCPUNAME - 1);
	while (*trimmed_string == ' ' || *trimmed_string == '\0') {
		trimmed_string--;
	}
	*trimmed_string = '\0';

	//trim  start
	trimmed_string = in_string;
	while (*trimmed_string == ' ') {
		trimmed_string++;
	}

	return trimmed_string;
}
