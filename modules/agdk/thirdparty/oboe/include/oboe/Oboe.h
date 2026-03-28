// Oboe stub header for compilation without vendored libraries.
// Replace with actual Oboe headers when available.
// See: https://github.com/google/oboe

#ifndef OBOE_OBOE_H
#define OBOE_OBOE_H

#include <cstdint>
#include <memory>

namespace oboe {

enum class Direction {
	Output,
	Input,
};

enum class PerformanceMode {
	None,
	PowerSaving,
	LowLatency,
};

enum class SharingMode {
	Exclusive,
	Shared,
};

enum class AudioFormat {
	Invalid = -1,
	Unspecified = 0,
	I16 = 1,
	Float = 2,
};

enum class ChannelCount {
	Unspecified = 0,
	Mono = 1,
	Stereo = 2,
};

enum class Result {
	OK,
	ErrorBase = -900,
	ErrorDisconnected,
	ErrorIllegalArgument,
	ErrorInternal,
	ErrorInvalidState,
	ErrorTimeout,
	ErrorWouldBlock,
};

enum class DataCallbackResult {
	Continue,
	Stop,
};

const char *convertToText(Result result);

class AudioStream {
public:
	virtual ~AudioStream() = default;
	virtual int32_t getSampleRate() const { return 44100; }
	virtual int32_t getFramesPerBurst() const { return 256; }
	virtual Result requestStart() { return Result::OK; }
	virtual Result requestStop() { return Result::OK; }
	virtual Result requestPause() { return Result::OK; }
	virtual Result close() { return Result::OK; }
};

class AudioStreamDataCallback {
public:
	virtual ~AudioStreamDataCallback() = default;
	virtual DataCallbackResult onAudioReady(
			AudioStream *stream,
			void *audioData,
			int32_t numFrames) = 0;
};

typedef std::unique_ptr<AudioStream> ManagedStream;

class AudioStreamBuilder {
public:
	AudioStreamBuilder *setDirection(Direction direction) { return this; }
	AudioStreamBuilder *setPerformanceMode(PerformanceMode mode) { return this; }
	AudioStreamBuilder *setSharingMode(SharingMode mode) { return this; }
	AudioStreamBuilder *setFormat(AudioFormat format) { return this; }
	AudioStreamBuilder *setChannelCount(ChannelCount count) { return this; }
	AudioStreamBuilder *setSampleRate(int32_t rate) { return this; }
	AudioStreamBuilder *setDataCallback(AudioStreamDataCallback *callback) { return this; }

	Result openManagedStream(ManagedStream &stream) {
		stream = std::make_unique<AudioStream>();
		return Result::OK;
	}
};

} // namespace oboe

#endif // OBOE_OBOE_H
