/* libLDR: Portable and easy-to-use LDraw format abstraction & I/O reference library *
 * To obtain more information about LDraw, visit http://www.ldraw.org.               *
 * Distributed in terms of the GNU Lesser General Public License v3                  *
 *                                                                                   *
 * Author: (c)2006-2013 Park "segfault" J. K. <mastermind_at_planetmono_dot_org>     */

#include "libldr/bit_buffer.h"

bit_buffer &bit_buffer::write_byte(const ubyte_t& data) {
    return this->write_<ubyte_t>(data);
}

bit_buffer &bit_buffer::write_char(const char& data) {
    return this->write_<char>(data);
}

bit_buffer &bit_buffer::write_bool(const bool& data) {
    return this->write_<bool>(data);
}

bit_buffer &bit_buffer::write_short(const short& data) {
    return this->write_<short>(data);
}

bit_buffer &bit_buffer::write_int(const uint32_t& data) {
    return this->write_<uint32_t>(data);
}

bit_buffer &bit_buffer::write_long(const uint64_t& data) {
    return this->write_<uint64_t>(data);
}

uint8_t bit_buffer::read_byte(const size_t byte_index) {
    return this->read_bytes_(byte_index, 1);
}

uint32_t bit_buffer::read_bytes(const size_t byte_index, const size_t num_bytes) {
    return this->read_bytes_(byte_index, num_bytes);
}

uint8_t bit_buffer::read_bit(const size_t bit_index) {
    return this->read_bits_(bit_index, 1, 0);
}

uint32_t bit_buffer::read_bits(const size_t bit_index, const size_t num_bits) {
    return this->read_bits_(bit_index, num_bits, 0);
}

bit_iterator bit_buffer::create_iter() const {
    return bit_iterator(*this);
}

bit_buffer::iterator bit_buffer::begin() {
    return bit_iterator(*this);
}

bit_buffer::const_iterator bit_buffer::begin() const {
    return bit_iterator(*this);
}

bit_buffer::iterator bit_buffer::end() {
    return bit_iterator(*this, this->buffer_.size() * 8);
}

bit_buffer::const_iterator bit_buffer::end() const {
    return bit_iterator(*this, this->buffer_.size() * 8);
}
