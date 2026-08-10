#pragma once

// CDuplicateDoc document

class CDuplicateDoc : public CDocument
{
protected:
	CDuplicateDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CDuplicateDoc)

public:
	virtual ~CDuplicateDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual BOOL OnNewDocument();

	DECLARE_MESSAGE_MAP()
};