// HexRichEdit.cpp : implementation file
//

#include "stdafx.h"
#include "hexvis.h"
#include "HexRichEdit.h"
#include "hexvisDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// HexRichEdit

HexRichEdit::HexRichEdit()
: parent(NULL)
{
}

HexRichEdit::~HexRichEdit()
{
}


BEGIN_MESSAGE_MAP(HexRichEdit, CRichEditCtrl)
	//{{AFX_MSG_MAP(HexRichEdit)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// HexRichEdit message handlers

void HexRichEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	switch(nChar) {
	case VK_LEFT:
		if(parent != NULL)
			parent->PrevByte();
		return;
	case VK_UP:
		if(parent != NULL)
			parent->PrevLine();
		return;
	case VK_RIGHT:
		if(parent != NULL)
			parent->NextByte();
		return;
	case VK_DOWN:
		if(parent != NULL)
			parent->NextLine();
		return;
	}
	
	CRichEditCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void HexRichEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch(nChar) {
	case VK_LEFT:
	case VK_UP:
	case VK_RIGHT:
	case VK_DOWN:
		return;	/* just make sure these dont get processed */
	}
	// TODO: Add your message handler code here and/or call default
	
	CRichEditCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
}
