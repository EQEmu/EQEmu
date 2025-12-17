/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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

template<class T>
class MyQueue;

template<class T>
class MyQueueNode
{
public:
	MyQueueNode(T* data)
	{
		next = 0;
		this->data = data;
	}

	friend class MyQueue<T>;

private:
	T* data;
	MyQueueNode<T>* next;
};

template<class T>
class MyQueue
{
public:
	MyQueue()
	{
		head = tail = 0;
	}
	~MyQueue() {
		clear();
	}

	void push(T* data)
	{
		if (head == 0)
		{
			tail = head = new MyQueueNode<T>(data);
		}
		else
		{
			tail->next = new MyQueueNode<T>(data);
			tail = tail->next;
		}
	}

	T* pop()
	{
		if (head == 0)
		{
			return 0;
		}

		T* data = head->data;
		MyQueueNode<T>* next_node = head->next;
		delete head;
		head = next_node;

		return data;
	}

	T* top()
	{
		if (head == 0)
		{
			return 0;
		}

		return head->data;
	}

	bool empty()
	{
		if (head == 0)
		{
			return true;
		}

		return false;
	}

	void clear()
	{
		T* d = 0;
		while((d = pop())) {
			delete d;
		}
		return;
	}

	int count()
	{
		int count = 0;
		MyQueueNode<T>* d = head;
		while(d != 0) {
			count++;
			d = d->next;
		}
		return(count);
	}

private:
	MyQueueNode<T>* head;
	MyQueueNode<T>* tail;
};
