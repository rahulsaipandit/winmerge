#include "stdafx.h"
#include "DuplicateDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDuplicateDoc

IMPLEMENT_DYNCREATE(CDuplicateDoc, CDocument)

CDuplicateDoc::CDuplicateDoc()
{
}

CDuplicateDoc::~CDuplicateDoc()
{
}

BOOL CDuplicateDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

// CDuplicateDoc diagnostics

#ifdef _DEBUG
void CDuplicateDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDuplicateDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

BEGIN_MESSAGE_MAP(CDuplicateDoc, CDocument)
END_MESSAGE_MAP()