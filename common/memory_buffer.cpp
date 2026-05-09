/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "memory_buffer.h"


EQ::MemoryBuffer::MemoryBuffer()
{
	buffer_ = nullptr;
	size_ = 0;
	capacity_ = 0;
	read_pos_ = 0;
	write_pos_ = 0;
}

EQ::MemoryBuffer::MemoryBuffer(size_t sz)
{
	buffer_ = nullptr;
	size_ = 0;
	capacity_ = 0;
	read_pos_ = 0;
	write_pos_ = 0;
	Resize(sz);
}

EQ::MemoryBuffer::MemoryBuffer(const MemoryBuffer &other)
{
	if(other.capacity_) {
		buffer_ = new uchar[other.capacity_];
		memcpy(buffer_, other.buffer_, other.capacity_);
	} else {
		buffer_ = nullptr;
	}

	size_ = other.size_;
	capacity_ = other.capacity_;
	write_pos_ = other.write_pos_;
	read_pos_ = other.read_pos_;
}

EQ::MemoryBuffer::MemoryBuffer(MemoryBuffer &&other)
{
	uchar *tbuf = other.buffer_;
	size_t tsz = other.size_;
	size_t tcapacity = other.capacity_;
	size_t twrite_pos = other.write_pos_;
	size_t tread_pos = other.read_pos_;

	other.buffer_ = nullptr;
	other.size_ = 0;
	other.capacity_ = 0;
	other.read_pos_ = 0;
	other.write_pos_ = 0;

	buffer_ = tbuf;
	size_ = tsz;
	capacity_ = tcapacity;
	write_pos_ = twrite_pos;
	read_pos_ = tread_pos;
}

EQ::MemoryBuffer& EQ::MemoryBuffer::operator=(const MemoryBuffer &other)
{
	if(this == &other) {
		return *this;
	}

	if(buffer_) {
		delete[] buffer_;
	}

	if(other.capacity_) {
		buffer_ = new uchar[other.capacity_];
		memcpy(buffer_, other.buffer_, other.capacity_);
	}
	else {
		buffer_ = nullptr;
	}

	size_ = other.size_;
	capacity_ = other.capacity_;
	write_pos_ = other.write_pos_;
	read_pos_ = other.read_pos_;
	return *this;
}

EQ::MemoryBuffer& EQ::MemoryBuffer::operator=(MemoryBuffer &&other)
{
	uchar *tbuf = other.buffer_;
	size_t tsz = other.size_;
	size_t tcapacity = other.capacity_;
	size_t twrite_pos = other.write_pos_;
	size_t tread_pos = other.read_pos_;

	other.buffer_ = nullptr;
	other.size_ = 0;
	other.capacity_ = 0;
	other.read_pos_ = 0;
	other.write_pos_ = 0;

	buffer_ = tbuf;
	size_ = tsz;
	capacity_ = tcapacity;
	write_pos_ = twrite_pos;
	read_pos_ = tread_pos;
	return *this;
}

EQ::MemoryBuffer& EQ::MemoryBuffer::operator+=(const MemoryBuffer &rhs)
{
	if(!rhs.buffer_) {
		return *this;
	}

	if(buffer_) {
		size_t old_size = size_;
		Resize(size_ + rhs.size_);
		memcpy(&buffer_[old_size], rhs.buffer_, rhs.size_);
	} else {
		buffer_ = new uchar[rhs.capacity_];
		memcpy(buffer_, rhs.buffer_, rhs.capacity_);
		size_ = rhs.size_;
		capacity_ = rhs.capacity_;
	}

	return *this;
}

EQ::MemoryBuffer::~MemoryBuffer()
{
	Clear();
}

uchar& EQ::MemoryBuffer::operator[](size_t pos)
{
	return buffer_[pos];
}

const uchar& EQ::MemoryBuffer::operator[](size_t pos) const
{
	return buffer_[pos];
}

bool EQ::MemoryBuffer::Empty()
{
	return size_ == 0;
}

bool EQ::MemoryBuffer::Empty() const
{
	return size_ == 0;
}

size_t EQ::MemoryBuffer::Size()
{
	return size_;
}

size_t EQ::MemoryBuffer::Size() const
{
	return size_;
}

size_t EQ::MemoryBuffer::Capacity()
{
	return capacity_;
}

size_t EQ::MemoryBuffer::Capacity() const
{
	return capacity_;
}

void EQ::MemoryBuffer::Resize(size_t sz)
{
	if(!buffer_) {
		size_t new_size = sz + 64;
		buffer_ = new uchar[new_size];
		capacity_ = new_size;
		size_ = sz;
		memset(buffer_, 0, capacity_);
		return;
	}

	if(sz > capacity_) {
		size_t new_size = sz + 32;
		uchar *temp = new uchar[new_size];
		memcpy(temp, buffer_, capacity_);
		delete[] buffer_;
		buffer_ = temp;
		
		capacity_ = new_size;
		size_ = sz;
	}
	else {
		size_ = sz;
	}
}

void EQ::MemoryBuffer::Clear()
{
	if(buffer_) {
		delete[] buffer_;
		buffer_ = nullptr;
	}
	
	size_ = 0;
	capacity_ = 0;
	write_pos_ = 0;
	read_pos_ = 0;
}

void EQ::MemoryBuffer::Zero()
{
	if(buffer_) {
		memset(buffer_, 0, capacity_);
	}
}

void EQ::MemoryBuffer::Write(const char *val, size_t len)
{
	size_t size_needed = write_pos_ + len;
	Resize(size_needed);

	memcpy(&buffer_[write_pos_], val, len);
	write_pos_ += len;
}

void EQ::MemoryBuffer::Read(uchar *buf, size_t len)
{
	memcpy(buf, &buffer_[read_pos_], len);
	read_pos_ += len;
}

void EQ::MemoryBuffer::Read(char *str)
{
	size_t len = strlen((const char*)&buffer_[read_pos_]);
	memcpy(str, &buffer_[read_pos_], len);
	read_pos_ += len;
}

void EQ::OutBuffer::overwrite(OutBuffer::pos_type position, const char *_Str, std::streamsize _Count)
{
	auto last_pos = tellp();
	seekp(position);
	write(_Str, _Count);
	seekp(last_pos);
}

uchar* EQ::OutBuffer::detach()
{
	size_t buffer_size = tellp();
	if (buffer_size == 0)
		return nullptr;

	auto out_buffer = new uchar[buffer_size];
	memcpy(out_buffer, str().c_str(), buffer_size);
	flush();

	return out_buffer;
}
