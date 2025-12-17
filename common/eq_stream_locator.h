/*
	Copyright (C) 2005 EQEmulator Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#pragma once

#include "common/types.h"

#include <map>
#include <string>

/*
This did not turn out nearly as nice as I hoped.
*/

class EQStreamInfo {
public:
	EQStreamInfo() {}
	EQStreamInfo(uint32 isrc_ip, uint32 idst_ip, uint16 isrc_port, uint16 idst_port) {
		src_ip = isrc_ip;
		dst_ip = idst_ip;
		src_port = isrc_port;
		dst_port = idst_port;
	}
	void invert(EQStreamInfo &r) const {
		r.src_ip = dst_ip;
		r.dst_ip = src_ip;
		r.src_port = dst_port;
		r.dst_port = src_port;
	}
	uint32 src_ip;
	uint32 dst_ip;
	uint16 src_port;
	uint16 dst_port;
};

inline bool operator<(const EQStreamInfo &l, const EQStreamInfo &r) {
/*printf("Less than called with:\n");
printf("0x%.8x:%d -> 0x%.8x:%d < \n", l.src_ip, l.src_port, l.dst_ip, l.dst_port);
printf("0x%.8x:%d -> 0x%.8x:%d ", r.src_ip, r.src_port, r.dst_ip, r.dst_port);

bool res;
if(l.src_ip != r.src_ip)
	res = (l.src_ip < r.src_ip);
else if(l.dst_ip != r.dst_ip)
	res = (l.dst_ip < r.dst_ip);
else if(l.src_port != r.src_port)
	res = (l.src_port < r.src_port);
else
res = (l.dst_port < r.dst_port);
if(res)
	printf(": True\n");
else
	printf(": False\n");*/


	if(l.src_ip != r.src_ip)
		return(l.src_ip < r.src_ip);
	if(l.dst_ip != r.dst_ip)
		return(l.dst_ip < r.dst_ip);
	if(l.src_port != r.src_port)
		return(l.src_port < r.src_port);
	return(l.dst_port < r.dst_port);

/*	//so, this turned out uglier than I had hoped
	if(l.src_ip < r.src_ip)
		return(true);
	if(l.src_ip > r.src_ip)
		return(false);
	if(l.dst_ip < r.dst_ip)
		return(true);
	if(l.dst_ip > r.dst_ip)
		return(false);
	if(l.src_port < r.src_port)
		return(true);
	if(l.src_port > r.src_port)
		return(false);
	return(l.dst_port < r.dst_port);*/
}

inline bool operator==(const EQStreamInfo &l, const EQStreamInfo &r) {
//	if(l.src_ip == r.dest_ip) {
//		//maybe swapped
//		return(l.src_port == r.dst_port && l.dst_ip == r.src_ip && l.dst_port == r.src_port);
//	}
	return(l.src_ip == r.src_ip && l.src_port == r.src_port && l.dst_ip == r.dst_ip && l.dst_port == r.dst_port);
}

//Forces the pointer T thing so we can return nullptr
template <class T>
class EQStreamLocator {
protected:
	typedef typename std::map<const EQStreamInfo, T *>::iterator iterator;
public:

	void Clear() {
		streams.clear();
	}

	void AddStream(const EQStreamInfo &i, T *o) {
		//do we care to check if it exists?

		//add this stream, and its inverse
		streams[i] = o;
		EQStreamInfo inv;
		i.invert(inv);
		streams[inv] = o;
	}

	//deletes this stream, and its inverse
	void RemoveStream(const EQStreamInfo &i) {
		iterator res;
		res = streams.find(i);
		if(res != streams.end())
			streams.erase(res);

		EQStreamInfo inv;
		i.invert(inv);
		res = streams.find(inv);
		if(res != streams.end())
			streams.erase(res);
	}

	//removes every occurance of this stream from the list
	void RemoveStream(T *it) {
		iterator cur, end;
		cur = streams.begin();
		end = streams.end();
		for(; cur != end; ++cur) {
			if(cur->second == it) {
				streams.erase(cur);
				//lazy recursive delete for now, since we have to redo
				//our iterators anyways
				RemoveStream(it);
				return;
			}
		}
	}

	T *GetStream(const EQStreamInfo &i) {
		iterator res;
		res = streams.find(i);
		//possibly optimization would be to store streams.end(), since it
		//may not be a constant time operation in theory, and update our
		//stored copy only on insert or delete
		if(res == streams.end())
			return(nullptr);
		return(res->second);
	}

	//allow people to iterate over the const struct
//	typedef map<const EQStreamInfo, T *>::const_iterator iterator;
//	inline iterator begin() const { return(streams.begin()); }
//	inline iterator end() const { return(streams.end()); }

protected:
	std::map<const EQStreamInfo, T *> streams;
};
