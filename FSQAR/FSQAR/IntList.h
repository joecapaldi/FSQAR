
// ======================================================================= 
// Flight Simulator Quick Access Recorder and Analyzer (FSQAR)
// -----------------------------------------------------------------------
// 
// Created by AlexShag, 2016
//
// This program is free software: you can redistribute it and/or modify 
// it under the terms of the GNU GPL (see License.txt).
//
// WARNING:
// This is a very bad code! This is not C++ code but "C with objects" one. 
// It breaks many basic principles of Objected-Oriented Programming.  
// It is badly designed and ugly written. I don’t advise to use it directly 
// without any modifications. This code may be used only as a starting point 
// for developing more advanced application.
//
// See more details in Readme.txt
// 
// =======================================================================


#ifndef __INTLIST_H__
#define __INTLIST_H__


//=======================================================================
//				List of integer values
//=======================================================================

class IntList
{
	struct ListItem
	{
		int item;
		ListItem* next;

		ListItem(int i, ListItem* n = 0) { item = i; next = n; }
	};

	int count;
	ListItem* first;
	ListItem* last;

public:

	IntList() : count(0), first(0), last(0){}

	IntList(const IntList& src)
	{
		count = 0;
		first = last;
		addLast(src);
	}

	~IntList()
	{
		if (count)
		{
			ListItem* current = 0;
			ListItem* next = first;
			while(next)
			{
				current = next;
				next = next->next;

				delete current;
			}
		}
	}

	int head() const { return first->item; }
	int& head() { return first->item; }

	int tail() const { return last->item; }
	int& tail(){ return last->item; }	

	int getCount() const { return count; }

	//----------------------------------------------------------------
	void addFirst(int item)
	{
		ListItem* newItem = new ListItem(item, first);
		if (first == 0)
		{	
			last = newItem;
		}

		first = newItem;
		count++;
	}

	//----------------------------------------------------------------
	void addLast(int item)
	{
		ListItem* newItem = new ListItem(item);
		if (first == 0)
		{	
			last = newItem;
		}
		else
		{
			last->next = newItem;
		}

		last = newItem;
		count++;
	}

	//----------------------------------------------------------------
	void addLast(const IntList& src)
	{
		for (ListItem* cur = src.first; cur ; cur = cur->next)
			addLast(cur->item);
	}

	//----------------------------------------------------------------
	void insert(int n)
	{
		ListItem* prev = 0;
		ListItem* succ = first;

		while (succ != 0 && succ->item < n)
		{
			prev = succ;
			succ = succ->next;

		}

		ListItem* newItem = new ListItem(n, succ);

		if (succ == 0)
			last = newItem;

		if (prev == 0)
			first = newItem;
		else
			prev->next = newItem;

		count++;
	}

	//----------------------------------------------------------------
	int remove()
	{
		int res = first->item;
		first = first->next;
		count--;

		return res;
	}

	//----------------------------------------------------------------
	int remove(int n)
	{

		ListItem* prev = 0;
		ListItem* current = first;

		while (current)
		{
			if (current->item == n)
			{
				if (prev)
					prev->next = current->next;

				if (current == last)
					last = prev;

				delete current;
				count--;

				return 0;
			}
			else
			{
				prev = current;
				current = current->next;
			}

		}

		return 0;
	}

	//----------------------------------------------------------------
	int getAt(int pos) const 
	{
		ListItem* current = first;

		for (int i = 0; i < pos; i++)
		{
			current = current->next;
		}
		return current->item;
	}
	//----------------------------------------------------------------

};

#endif // __INTLIST_H__
