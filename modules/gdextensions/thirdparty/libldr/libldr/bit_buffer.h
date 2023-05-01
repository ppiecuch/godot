/* libLDR: Portable and easy-to-use LDraw format abstraction & I/O reference library *
 * To obtain more information about LDraw, visit http://www.ldraw.org.               *
 * Distributed in terms of the GNU Lesser General Public License v3                  *
 *                                                                                   *
 * Author: (c)2006-2009 Park "segfault" J. K. <mastermind_at_planetmono_dot_org>     */

#ifndef LIBLDR_BIT_BUFFER_H
#define LIBLDR_BIT_BUFFER_H

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include <vector>
#include <map>
#include <memory>

#include <iostream>

#define DEFAULT_SIZE 1024

class bit_iterator;

/*
 * Byte buffer is a class that lets you easily serialize data
 * into the form of bits and bytes, and stores it all in a
 * byte array, but is accessed publicly through the read_bit(s)
 * and read_byte(s) methods.
 *
 * Example:
 *      bit_buffer bb;
 *
 *      bb.write_bit(true);
 *      bb.write_bits(2, 2);
 *      bb.write_bit(false);
 *      bb.write_bits(10, 4);
 *      bb.write_int(100);
 *      bb.write_byte(8);
 *
 *      std::cout << bb.read_bit(0) << std::endl;       // Outputs 1 (true)
 *      std::cout << bb.read_bits(1, 2) << std::endl;   // Outputs 2
 *      std::cout << bb.read_bit(3) << std::endl;       // Outputs 0 (false)
 *      std::cout << bb.read_bits(4, 4) << std::endl;   // Outputs 10
 *      std::cout << bb.read_bytes(1, 4) << std::endl;  // Outputs 100
 *      std::cout << bb.read_byte(5) << std::endl;      // Outputs 8
 */
class bit_buffer {
	friend class bit_iterator;

	typedef bit_iterator iterator;
	typedef const bit_iterator const_iterator;

protected:
	/* Private type definitions */
	typedef uint8_t ubyte_t;
	typedef std::vector<ubyte_t> bytes_t;

	/* Array of bytes that handles the storage of all bytes for this class */
	bytes_t buffer_;
	uint32_t pos_;
	uint8_t bit_index_;

	/* Appends the data parameter's bits to the buffer (bytes array) */
	bit_buffer &write_bits_(const uint32_t data, const size_t bits) {
		static constexpr uint8_t bits_per_byte = 8;
		static constexpr uint32_t mask_all = 0xffffffff;

		if (this->bit_index_ == 0 && bits > 0) {
			this->buffer_.push_back(0);
		}

		/* clear unused bits (for safty)  TODO: check for overflow */
		const uint32_t safe = data & (mask_all >> (32-bits));

		/* Not enough bits in this byte to write all the bits required */
		if (this->bit_index_ + bits > bits_per_byte) {
			const uint8_t offset = bits_per_byte - this->bit_index_;
			const uint8_t remainder_bits = bits - offset;
			const uint32_t remainder_value = (safe & (mask_all >> (32-remainder_bits)));

			this->buffer_[this->pos_] |= (safe >> remainder_bits);

			this->bit_index_ = 0;
			this->pos_++;

			/* Process the remaining bits in the next byte */
			this->write_bits_(remainder_value, remainder_bits);

			return *this;
		}

		const uint8_t offset = bits_per_byte - this->bit_index_ - bits;
		this->buffer_[this->pos_] |= (safe << offset);
		this->bit_index_ += bits;

		if (this->bit_index_ == bits_per_byte)
		{
			this->bit_index_ = 0;
			this->pos_++;
		}

		return *this;
	}

	template <typename T> inline bit_buffer &write_(const T& data) {
		size_t bytes = sizeof(T);
		return this->write_bytes_(data, bytes);
	}

	template <typename T> inline bit_buffer &write_bytes_(const T& data, size_t bytes) {
		const uint32_t temp = static_cast<uint32_t>(data);

		/* If beginning of a byte, then it can just push bytes */
		if (this->bit_index_ == 0)
		{
			int i = bytes - 1;
			while (i >= 0) {
				this->buffer_.push_back(static_cast<ubyte_t>((temp >> (i * 8))));
				--i;
			}
			this->pos_ += bytes;
			return *this;
		}

		/* If the current byte has a few bits written to it already, but */
		/* not all eight bits, then it must process the data bit by bit  */
		return this->write_bits_(data, bytes * 8);
	}

	uint32_t read_bits_(const size_t bit_index, const size_t num_bits, size_t ret) const
	{
		if (bit_index + num_bits > this->buffer_.size() * 8)
		{
			throw;
		}

		uint32_t pos = static_cast<uint32_t>(bit_index / 8);
		uint8_t bit_index_start = static_cast<uint8_t>(bit_index - (pos * 8));
		uint32_t bit_index_end = bit_index_start + num_bits - 1;

		// If we exceeded the number of bits that can be read
		// from this byte, then move to the next byte and
		// continue reading the bits from that byte
		if (bit_index_end >= 8)
		{
			ubyte_t byte = this->buffer_[pos];
			int offset = 8 - num_bits - bit_index_start;
			if (offset < 0)
			{
				ubyte_t mask = (0xff >> bit_index_start);
				byte &= mask;
			} else {
				byte >>= offset;
			}

			//ret += byte;
			uint32_t bits_read = 8 - bit_index_start;
			uint32_t p = num_bits - bits_read;
			offset = 0;
			while (p < num_bits)
			{
				ret += static_cast<uint32_t>(((byte >> offset) & 0x01) * pow(2, p));
				++p;
				++offset;
			}

			return read_bits_(bit_index + bits_read, num_bits - bits_read, ret);
		}


		/* Remove everything in front of the starting bit */
		ubyte_t byte = this->buffer_[pos];
		if (bit_index_start > 0)
		{
			ubyte_t mask = ~(0xff << (8 - bit_index_start));
			byte &= mask;
		}

		byte >>= (8 - num_bits - bit_index_start);
		ret += static_cast<uint32_t>(byte);

		return ret;
	}

	inline uint32_t read_bytes_(const size_t byte_index, const size_t num_bytes) {
		if (byte_index + num_bytes > this->buffer_.size()) {
			throw;
		}

		return this->read_bits_(byte_index * 8, num_bytes * 8, 0);
	}

public:
	/* The methods below write the data to the byte array */
	template <typename T> inline bit_buffer &write(const T& data) {
		return this->write_<T>(data);
	}

	template <typename T> inline bit_buffer &write_bits(const T& data, const size_t num_bits) {
		return this->write_bits_(static_cast<uint32_t>(data), num_bits);
	}

	bit_buffer &write_byte(const ubyte_t& data);
	bit_buffer &write_char(const char& data);
	bit_buffer &write_bool(const bool& data);
	bit_buffer &write_short(const short& data);
	bit_buffer &write_int(const uint32_t& data);
	bit_buffer &write_long(const uint64_t& data);

	/* Returns the value of the bytes starting at byte_index and           */
	/* ending at (byte_index + num_bytes - 1)                              */
	/*                                                                     */
	/* For example:                                                        */
	/*      bit_buffer bf;                                                 */
	/*      // If two bytes were written to the buffer                     */
	/*      bf.write_byte(10);                                             */
	/*      bf.write_int(20); // 4 bytes                                   */
	/*                                                                     */
	/*      std::cout << bf.read_byte(0) << std::endl;   // Outputs 10     */
	/*      std::cout << bf.read_bytes(1, 4) << std::endl;   // Outputs 20 */
	uint8_t read_byte(const size_t byte_index);
	uint32_t read_bytes(const size_t byte_index, const size_t num_bytes);

	/* Returns the value of the bits starting at bit_index and            */
	/* ending at (bit_index + num_bits - 1)                               */
	/*                                                                    */
	/* For example:                                                       */
	/*      bit_buffer bf;                                                */
	/*      // If two bytes were written to the buffer                    */
	/*      bf.write_byte(10);                                            */
	/*      bf.write_byte(8);                                             */
	/*      bf.write_bits(5, 3);                                          */
	/*                                                                    */
	/*      std::cout << bf.read_bits(0, 4) << std::endl;   // Outputs 10 */
	/*      std::cout << bf.read_bits(4, 4) << std::endl;   // Outputs 8  */
	/*      std::cout << bf.read_bits(8, 3) << std::endl;   // Outputs 5  */
	uint8_t read_bit(const size_t bit_index);
	uint32_t read_bits(const size_t bit_index, const size_t num_bits);

	inline const bytes_t get_bytes() const { return this->buffer_; }
	inline size_t get_bit_pos() const { return this->pos_*8 + this->bit_index_; }
	inline void set_bit_pos(size_t pos) { this->pos_=pos/8, this->bit_index_=pos%8; }
	bit_iterator create_iter() const;

	/* For each loop iterator */
	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;

	/* Operator overrides */
	bool operator==(const bit_buffer &other) {
		return this->buffer_ == other.buffer_;
	}

	bool operator!=(const bit_buffer &other) {
		return this->buffer_ != other.buffer_;
	}

	/* Debug options */
	bool verbose_ = false;

	bit_buffer(const size_t size = DEFAULT_SIZE) : pos_(0), bit_index_(0)
	{
		this->buffer_.reserve(size);
	}
	virtual ~bit_buffer() {}
};

class bit_iterator {
	bit_buffer buffer_;
	size_t bit_index_;

public:
	inline uint8_t current_bit() const {
		return static_cast<uint8_t>(this->buffer_.read_bits_(this->bit_index_, 1, 0));
	}

	bit_iterator &operator++() {
		++this->bit_index_;
		return *this;
	}

	bit_iterator operator++(int) {
		bit_iterator temp = *this;
		++*this;
		return temp;
	}

	bool operator==(const bit_iterator &other) {
		return this->bit_index_ == other.bit_index_;
	}

	bool operator!=(const bit_iterator &other) {
		return this->bit_index_ != other.bit_index_;
	}

	uint8_t operator*() {
		return this->current_bit();
	}

	uint8_t operator*() const {
		return this->current_bit();
	}

	explicit bit_iterator(const bit_buffer& buffer) :
		buffer_(buffer), bit_index_(0)
	{}

	bit_iterator(const bit_buffer& buffer, const size_t bit_index) :
		buffer_(buffer), bit_index_(bit_index)
	{}
};

/* Simplified, read-only, version. Works only with pre-allocated buffer, pass in constructor. */
template<typename FType> class t_bit_buffer_ro {
protected:
	typedef uint8_t ubyte_t;

	/* Array of bytes that handles the storage of all bytes for this class */
	FType &buffer_func_;
	size_t buffer_size_;
	size_t bit_pos_;

	uint32_t read_bits_(const size_t bit_index, const size_t num_bits, size_t ret) const
	{
		if (bit_index + num_bits > buffer_size_ * 8)
		{
			throw;
		}

		uint32_t pos = static_cast<uint32_t>(bit_index / 8);
		uint8_t bit_index_start = static_cast<uint8_t>(bit_index - (pos * 8));
		uint32_t bit_index_end = bit_index_start + num_bits - 1;

		// If we exceeded the number of bits that can be read
		// from this byte, then move to the next byte and
		// continue reading the bits from that byte
		if (bit_index_end >= 8)
		{
			ubyte_t byte = buffer_func_(pos);
			int offset = 8 - num_bits - bit_index_start;
			if (offset < 0)
			{
				ubyte_t mask = (0xff >> bit_index_start);
				byte &= mask;
			} else {
				byte >>= offset;
			}

			//ret += byte;
			uint32_t bits_read = 8 - bit_index_start;
			uint32_t p = num_bits - bits_read;
			offset = 0;
			while (p < num_bits)
			{
				ret += static_cast<uint32_t>(((byte >> offset) & 0x01) * pow(2, p));
				++p;
				++offset;
			}

			return read_bits_(bit_index + bits_read, num_bits - bits_read, ret);
		}


		/* Remove everything in front of the starting bit */
		ubyte_t byte = buffer_func_(pos);
		if (bit_index_start > 0)
		{
			ubyte_t mask = ~(0xff << (8 - bit_index_start));
			byte &= mask;
		}

		byte >>= (8 - num_bits - bit_index_start);
		ret += static_cast<uint32_t>(byte);

		return ret;
	}

	inline uint32_t read_bytes_(const size_t byte_index, const size_t num_bytes)
	{
		if (byte_index + num_bytes > buffer_size_)
		{
			throw;
		}
		return this->read_bits_(byte_index * 8, num_bytes * 8, 0);
	}

public:
	/* Returns the value of the bytes starting at byte_index and           */
	/* ending at (byte_index + num_bytes - 1)                              */
	/*                                                                     */
	/* For example:                                                        */
	/*      bit_buffer bf;                                                 */
	/*      // If two bytes were written to the buffer                     */
	/*      bf.write_byte(10);                                             */
	/*      bf.write_int(20); // 4 bytes                                   */
	/*                                                                     */
	/*      std::cout << bf.read_byte(0) << std::endl;   // Outputs 10     */
	/*      std::cout << bf.read_bytes(1, 4) << std::endl;   // Outputs 20 */
	uint8_t read_byte(const size_t byte_index);
	uint32_t read_bytes(const size_t byte_index, const size_t num_bytes);

	/* Returns the value of the bits starting at bit_index and            */
	/* ending at (bit_index + num_bits - 1)                               */
	/*                                                                    */
	/* For example:                                                       */
	/*      bit_buffer bf;                                                */
	/*      // If two bytes were written to the buffer                    */
	/*      bf.write_byte(10);                                            */
	/*      bf.write_byte(8);                                             */
	/*      bf.write_bits(5, 3);                                          */
	/*                                                                    */
	/*      std::cout << bf.read_bits(0, 4) << std::endl;   // Outputs 10 */
	/*      std::cout << bf.read_bits(4, 4) << std::endl;   // Outputs 8  */
	/*      std::cout << bf.read_bits(8, 3) << std::endl;   // Outputs 5  */
	uint8_t read_bit(const size_t bit_index);
	uint32_t read_bits(const size_t bit_index, const size_t num_bits)
	{
		uint32_t ret = this->read_bits_(bit_index, num_bits, 0);
		return ret;
	}

	uint32_t read_bits(const size_t num_bits)
	{
		uint32_t ret = this->read_bits_(bit_pos_, num_bits, 0);
		bit_pos_ += num_bits;
		return ret;
	}
	t_bit_buffer_ro &read_bits(const size_t num_bits, uint32_t &ret);

	inline size_t get_bit_pos() const { return this->bit_pos_; }
	inline void set_bit_pos(size_t pos) { this->bit_pos_=pos; }

	t_bit_buffer_ro(FType &buffer_func, size_t buffer_size = 0)
	: buffer_func_(buffer_func)
	, buffer_size_(buffer_size)
	, bit_pos_(0) {}
};

class bit_buffer_ro : public t_bit_buffer_ro<bit_buffer_ro> {
	friend class t_bit_buffer_ro;
protected:
	typedef const char *bytes_t;

	/* Array of bytes that handles the storage of all bytes for this class */
	bytes_t buffer_;
	inline uint8_t operator()(size_t pos) const {  return buffer_[pos]; }

public:
	bit_buffer_ro(const char *pre_allocated, const size_t size)
		: t_bit_buffer_ro(*this, size)
		, buffer_(pre_allocated) {}
	virtual ~bit_buffer_ro() {}
};

// mini-zip forward header:
#define ZEXPORT
typedef void *voidp;
typedef off_t z_off_t;
typedef struct gzFile_s *gzFile; /* semi-opaque gzip file descriptor */
#ifdef __cplusplus
extern "C" {
#endif
	gzFile ZEXPORT gzmemopen(const unsigned char *mem, size_t mem_size, const char *mode);
	z_off_t ZEXPORT gzseek(gzFile file, z_off_t offset, int whence);
	z_off_t ZEXPORT gztell(gzFile file);
	int ZEXPORT gzread(gzFile file, voidp buf, unsigned len);
	int ZEXPORT gzeof(gzFile file);
	int ZEXPORT gzclose(gzFile file);
#ifdef __cplusplus
}
#endif
// end.

class bit_buffer_gz : public t_bit_buffer_ro<bit_buffer_gz>
{
	friend class t_bit_buffer_ro;

protected:
	/* Array of bytes that handles the storage of all bytes for this class */
	gzFile buffer_file_;
	/* Cached data */
	static const size_t cache_size_ = 32768;
	uint8_t cache_[cache_size_];
	size_t cache_start_index_, cache_end_index_;

	inline uint8_t operator()(size_t pos) {
		if (pos >= cache_start_index_&& pos < cache_end_index_)
		{
			return cache_[pos-cache_start_index_];
		}
		// cache_ is cache_size_ aligned
		cache_start_index_ = int(pos/cache_size_) * cache_size_;
		gzseek(buffer_file_, cache_start_index_, SEEK_SET);
		cache_end_index_ = cache_start_index_ + gzread(buffer_file_, (voidp)cache_, cache_size_);
		return cache_[pos-cache_start_index_];
	}

public:
	bool eof() const { return gzeof(buffer_file_); }
	off_t seek(off_t ofs) { return gzseek(buffer_file_, ofs, SEEK_SET); }

	bit_buffer_gz(const unsigned char *pre_allocated, const size_t size)
		: t_bit_buffer_ro(*this, *(uint32_t*)(pre_allocated+size-sizeof(uint32_t)))
		, buffer_file_(gzmemopen(pre_allocated, size, "r")) {}
	virtual ~bit_buffer_gz() { gzclose(buffer_file_); }
};

#endif // LIBLDR_BIT_BUFFER_H
