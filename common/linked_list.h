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

#include "common/types.h"

enum direction{FORWARD,BACKWARD};

template<class TYPE> class LinkedListIterator;

template<class TYPE>
class ListElement
{
private:

	TYPE data;
	ListElement<TYPE>* next;
	ListElement<TYPE>* prev;
public:
	ListElement ();
	ListElement (const TYPE&);
	ListElement (const ListElement<TYPE>&);

	~ListElement ();

	ListElement<TYPE>&	operator= (const ListElement<TYPE>&);

	ListElement<TYPE>*	GetLast ()
	{
		ListElement<TYPE>* tmp = this;
		while (tmp->GetNext()) {
			tmp = tmp->GetNext();
		}
		return tmp;
	}
	ListElement<TYPE>*	GetNext () const { return next ; }
	ListElement<TYPE>*	GetPrev () const { return prev ; }

	inline TYPE&			GetData ()			{ return data ; }
	inline const TYPE&	GetData () const	{ return data ; }

	void		SetData ( const TYPE& d )			{ data = d ; } // Quagmire - this may look like a mem leak, but dont change it, this behavior is expected where it's called
	void		SetLastNext ( ListElement<TYPE>* p )
	{
		GetLast()->SetNext(p);
	}
	void		SetNext (ListElement<TYPE>* n)	{ next = n ; }
	void		SetPrev (ListElement<TYPE>* p)	{ prev = p ; }

	void		ReplaceData(const TYPE&);
};

template<class TYPE>
class LinkedList
{
private:
	uint32					count;
	ListElement<TYPE>*		first;
	bool					list_destructor_invoked;

public:

	LinkedList();
	~LinkedList();
	bool dont_delete;
	LinkedList<TYPE>&			operator= (const LinkedList<TYPE>&);

	void Append (const TYPE&);
	void Insert (const TYPE&);
	TYPE Pop();
	TYPE PeekTop();
	void Clear();
	void LCount() { count--; }
	void ResetCount() { count=0; }
	uint32	Count() { return count; }
	ListElement<TYPE>* GetFirst() { return first; }

	friend class LinkedListIterator<TYPE>;
};

template<class TYPE>
class LinkedListIterator
{
private:
	LinkedList<TYPE>& list;
	ListElement<TYPE>* current_element;
	direction dir;

public:
	LinkedListIterator(LinkedList<TYPE>& l,direction d = FORWARD) : list(l), dir(d) {};

	void Advance();
	const TYPE& GetData();
	bool IsFirst()
	{
		if (current_element->GetPrev() == 0)
			return true;
		else
			return false;
	}
	bool IsLast()
	{
		if (current_element->GetNext() == 0)
			return true;
		else
			return false;
	}
	bool MoreElements();
	void MoveFirst();
	void MoveLast();
	void RemoveCurrent(bool DeleteData = true);
	void Replace(const TYPE& new_data);
	void Reset();
	void SetDir(direction);
};

template<class TYPE>
void LinkedListIterator<TYPE>::Advance()
{
	if (current_element == 0)
	{
		return;
	}
	if (dir == FORWARD)
	{
		current_element = current_element->GetNext();
	}
	else
	{
		current_element = current_element->GetPrev();
	}

	if (list.list_destructor_invoked)
	{
		while(current_element && current_element->GetData() == 0)
		{
//			if (current_element == 0)
//			{
//				return;
//			}
			if (dir == FORWARD)
			{
				current_element = current_element->GetNext();
			}
			else
			{
				current_element = current_element->GetPrev();
			}
		}
	}
}

template<class TYPE>
bool LinkedListIterator<TYPE>::MoreElements()
{
	if (current_element == 0)
		return false;
	return true;
}

template<class TYPE>
const TYPE& LinkedListIterator<TYPE>::GetData()
{
	return current_element->GetData();
}

template<class TYPE>
void LinkedListIterator<TYPE>::MoveFirst()
{
	ListElement<TYPE>* prev = current_element->GetPrev();
	ListElement<TYPE>* next = current_element->GetNext();

	if (prev == 0)
	{
		return;
	}

	//if (prev != 0)
	//{
		prev->SetNext(next);
	//}
	if (next != 0)
	{
		next->SetPrev(prev);
	}
	current_element->SetPrev(0);
	current_element->SetNext(list.first);
	list.first->SetPrev(current_element);
	list.first = current_element;
}


template<class TYPE>
void LinkedListIterator<TYPE>::MoveLast()
{
	ListElement<TYPE>* prev = current_element->GetPrev();
	ListElement<TYPE>* next = current_element->GetNext();

	if (next == 0)
	{
		return;
	}

	if (prev != 0)
	{
		prev->SetNext(next);
	}
	else
	{
		list.first = next;
	}
	//if (next != 0)
	//{
		next->SetPrev(prev);
	//}
	current_element->SetNext(0);
	current_element->SetPrev(next->GetLast());
	next->GetLast()->SetNext(current_element);
}

template<class TYPE>
void LinkedListIterator<TYPE>::RemoveCurrent(bool DeleteData)
{
	ListElement<TYPE>* save;

	if (list.first == current_element)
	{
		list.first = current_element->GetNext();
	}

	if (current_element->GetPrev() != 0)
	{
		current_element->GetPrev()->SetNext(current_element->GetNext());
	}
	if (current_element->GetNext() != 0)
	{
		current_element->GetNext()->SetPrev(current_element->GetPrev());
	}
	if (dir == FORWARD)
	{
		save = current_element->GetNext();
	}
	else
	{
		save = current_element->GetPrev();
	}
	current_element->SetNext(0);
	current_element->SetPrev(0);
	if (!DeleteData)
		current_element->SetData(0);
	safe_delete(current_element);
	current_element = save;
	list.LCount();
}

template<class TYPE>
void LinkedListIterator<TYPE>::Replace(const TYPE& new_data)
{
	current_element->ReplaceData(new_data);
}

template<class TYPE>
void LinkedListIterator<TYPE>::Reset()
{
	if (dir == FORWARD)
	{
		current_element = list.first;
	}
	else
	{
		if (list.first == 0)
		{
			current_element = 0;
		}
		else
		{
			current_element = list.first->GetLast();
		}
	}

	if (list.list_destructor_invoked)
	{
		while(current_element && current_element->GetData() == 0)
		{
//			if (current_element == 0)
//			{
//				return;
//			}
			if (dir == FORWARD)
			{
				current_element = current_element->GetNext();
			}
			else
			{
				current_element = current_element->GetPrev();
			}
		}
	}
}

template<class TYPE>
void LinkedListIterator<TYPE>::SetDir(direction d)
{
	dir = d;
}

template<class TYPE>
ListElement<TYPE>::ListElement(const TYPE& d)
{
	data = d;
	next = 0;
	prev = 0;
}

template<class TYPE>
ListElement<TYPE>::~ListElement()
{
//	std::cout << "ListElement<TYPE>::~ListElement()" << std::endl;

	if (data != 0)
		safe_delete(data);
	data = 0;
	if (next != 0)
	{
		safe_delete(next);
		next = 0;
	}
}

template<class TYPE>
void ListElement<TYPE>::ReplaceData(const TYPE& new_data)
{
	if (data != 0)
		safe_delete(data);
	data = new_data;
}

template<class TYPE>
LinkedList<TYPE>::LinkedList()
{
	list_destructor_invoked = false;
	first = 0;
	count = 0;
	dont_delete = false;
}

template<class TYPE>
LinkedList<TYPE>::~LinkedList()
{
	list_destructor_invoked = true;
	if(!dont_delete)
		Clear();
}

template<class TYPE>
void LinkedList<TYPE>::Clear() {
	while (first) {
		ListElement<TYPE>* tmp = first;
		first = tmp->GetNext();
		tmp->SetNext(0);
		safe_delete(tmp);
	}
	ResetCount();
}

template<class TYPE>
void LinkedList<TYPE>::Append(const TYPE& data)
{
	ListElement<TYPE>* new_element = new ListElement<TYPE>(data);

	if (first == 0)
	{
		first = new_element;
	}
	else
	{
		new_element->SetPrev(first->GetLast());
		first->SetLastNext(new_element);
	}
	count++;
}

template<class TYPE>
void LinkedList<TYPE>::Insert(const TYPE& data)
{
	ListElement<TYPE>* new_element = new ListElement<TYPE>(data);

	new_element->SetNext(first);
	if (first != 0)
	{
		first->SetPrev(new_element);
	}
	first = new_element;
	count++;
}

template<class TYPE>
TYPE LinkedList<TYPE>::Pop() {
	TYPE ret = 0;
	if (first) {
		ListElement<TYPE>* tmpdel = first;
		first = tmpdel->GetNext();
		if (first)
			first->SetPrev(0);
		ret = tmpdel->GetData();
		tmpdel->SetData(0);
		tmpdel->SetNext(0);
		safe_delete(tmpdel);
		count--;
	}
	return ret;
}

template<class TYPE>
TYPE LinkedList<TYPE>::PeekTop() {
	if (first)
		return first->GetData();
	return 0;
}
