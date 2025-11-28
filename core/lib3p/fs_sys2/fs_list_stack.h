/* fs_list_stack.h - The CLfListStack is a special type of
	list that works as both a stack and a linked list.  It
	is designed to be used with anything, but all classes
	it contains must have the base type of LSItem.  This is
	designed to work both as a stack and as a linked list.
	In that way it has a broad range of uses.  It is was
	primarily designed to hold various lists of entities.
*/
#ifndef __FS_LIST_STACK_H__
#define __FS_LIST_STACK_H__

#include "fs_sys2.h"

#define FS_CACHE_ALIGN __declspec(align(16))

struct FS_SYS2_EXPORTS CLfListStack
{
public:
	//All items to be stored in the list stack must inherit from
	//CLfListStack::LSItem.  If they don't they can't be stored in
	//the list.
	FS_CACHE_ALIGN struct LSItem
	{
		LSItem* m_pNext;
		LSItem* m_pPrev;
		fs_dword m_nItemID;
		fs_dword m_nListStackID;
	};
private:
	//We keep track of the ID of this list, this ID is generated
	//to be unique from any other lists.
	const fs_dword m_nID;
public:
	//It is necessary to keep track of a few items in the list.
	//The list is designed to be transversed either way.
	//The members are public, so they can be accessed directly, for
	//transversals.
	LSItem* m_pFirst;
	LSItem* m_pLast;
	fs_dword m_nCount;
public:
	CLfListStack();
	~CLfListStack();
	LSItem* Pop();
	LSItem* Peek();
	void Push(LSItem* pNode);
	void Remove(LSItem* pNode);
	fs_bool IsEmpty()const;
	void Clear();
	void Init(LSItem* pList, fs_dword nCount, fs_dword nItemSize);
	
private:
	//We keep track of the next id to be used when another list is created,
	//this is incremented each time we create a new list.
	static fs_dword s_nNextID;
};

#endif __FS_LIST_STACK_H__