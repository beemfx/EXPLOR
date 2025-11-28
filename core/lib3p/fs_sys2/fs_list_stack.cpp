//See header file for instructions.
#include "fs_list_stack.h"

//We keep track of a ID identifier which
//is assigned to each list, that way we can
//distinguish lists, so that we can identify
//to what list a node belongs.
fs_dword CLfListStack::s_nNextID=811983;  


CLfListStack::CLfListStack():
	m_nID(s_nNextID++),
	m_pFirst(FS_NULL),
	m_pLast(FS_NULL),
	m_nCount(0)
{}

CLfListStack::~CLfListStack()
{}

CLfListStack::LSItem* CLfListStack::Pop()
{
	if(!m_pFirst)
		return FS_NULL;
	
	CLfListStack::LSItem* pRes=m_pFirst;
		
	//Removing the first node is a special case.
	m_pFirst=m_pFirst->m_pNext;
	//If the first node was also the last node then we also
	//need to set the last node to null.
	if(m_pFirst==FS_NULL)
	{
		m_pLast=FS_NULL;
	}
	else
	{
		//The first node's previous must be set to null.
		m_pFirst->m_pPrev=FS_NULL;
	}
	
	m_nCount--;
	pRes->m_nListStackID=0;
	return pRes;
}


CLfListStack::LSItem* CLfListStack::Peek()
{
	return m_pFirst;
}


void CLfListStack::Push(CLfListStack::LSItem* pNode)
{
	#if 0
	if(pNode->m_nListStackID!=0)
	{
		//Warning the node was still in a list.
	}
	#endif
	pNode->m_pPrev=FS_NULL;
	pNode->m_pNext=m_pFirst;
	pNode->m_nListStackID=m_nID;
	if(m_pFirst)
	{
		m_pFirst->m_pPrev=pNode;
	}
	else
	{
		m_pLast=pNode;
	}
	m_pFirst=pNode;
	m_nCount++;
}


fs_bool CLfListStack::IsEmpty()const
{		
	return m_pFirst?FS_FALSE:FS_TRUE;
}


void CLfListStack::Clear()
{
	for(LSItem* pNode=m_pFirst; pNode; pNode=pNode->m_pNext)
	{
		pNode->m_nListStackID=0;
	}
	m_pFirst=FS_NULL;
	m_pLast=FS_NULL;
	m_nCount=0;
}

void CLfListStack::Remove(CLfListStack::LSItem* pNode)
{
	//If this node wasn't even in this list, we just
	//return.
	if(pNode->m_nListStackID!=m_nID)
		return;
		
	if(pNode==m_pFirst)
	{
		//Removing the first node is a special case.
		m_pFirst=m_pFirst->m_pNext;
		//If the first node was also the last node then we also
		//need to set the last node to null.
		if(m_pFirst==FS_NULL)
		{
			m_pLast=FS_NULL;
		}
		else
		{
			//The first node's previous must be set to null.
			m_pFirst->m_pPrev=FS_NULL;
		}
	}
	else if(pNode==m_pLast)
	{
		//The last node is also a special case, but we know it isn't the only
		//node in the list because that would have been checked above.
		m_pLast=m_pLast->m_pPrev;
		m_pLast->m_pNext=FS_NULL;
	}
	else
	{
		//One problem here is that it isn't gauranteed that pNode
		//was actually in pList, but it is a quick way to remove
		//a node, the id check above should prevent weird removals.
		if(pNode->m_pPrev && pNode->m_pNext)
		{
			pNode->m_pPrev->m_pNext=pNode->m_pNext;
			pNode->m_pNext->m_pPrev=pNode->m_pPrev;
		}
	}
	m_nCount--;
}

void CLfListStack::Init(CLfListStack::LSItem* pList, fs_dword nCount, fs_dword nItemSize)
{
	for(fs_dword i=0; i<nCount; i++)
	{
		LSItem* pCurItem=(LSItem*)((fs_byte*)pList+i*nItemSize);
		pCurItem->m_nListStackID=0;
		pCurItem->m_nItemID=i;
		Push(pCurItem);
		/*
		pList[i].m_nListStackID=0;
		Push(&pList[i]);
		*/
	}
}