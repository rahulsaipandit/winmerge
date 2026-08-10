#pragma once

#include "DuplicateReviewDialog.h" // for shared logic
#include <memory>

class CDuplicateDoc;

// CDuplicateView form view

class CDuplicateView : public CFormView
{
	DECLARE_DYNCREATE(CDuplicateView)

protected:
	CDuplicateView();           // protected constructor used by dynamic creation
	virtual ~CDuplicateView();

public:
	enum { IDD = IDD_DUPLICATE_REVIEW };
	CDuplicateDoc* GetDocument(); // non-debug version is inline
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate();
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

public:
	// Shared logic from dialog
	std::unique_ptr<CDuplicateReviewDialogCache> m_cache;
	CString m_initialMaster;
	CString m_initialDuplicate;

	// Controls
	CStatic m_groups;
	CStatic m_duplicates;
	CStatic m_reclaim;
	CEdit m_masterFolder;
	CEdit m_duplicateFolder;
	CButton m_nameCompare;
	CButton m_sizeCompare;
	CButton m_timestampCompare;
	CButton m_contentCompare;
	CEdit m_ignorePattern;
	CListBox m_ignoreList;
	CTreeCtrl m_masterTree;
	CTreeCtrl m_duplicateTree;
	CComboBox m_thumbnailSize;

	// Methods
	void SetInitialFolders(const CString& master, const CString& duplicate);
	void UpdateStatistics(int groups, int duplicates, uint64_t reclaimBytes);
	void RepopulateTrees();
	void PerformComparison();

	// Handlers
	afx_msg void OnBnClickedRefresh();
	afx_msg void OnBnClickedBrowseMaster();
	afx_msg void OnBnClickedBrowseDuplicate();
	afx_msg void OnBnClickedAddPattern();
	afx_msg void OnBnClickedRemovePattern();
	afx_msg void OnBnClickedCompare();
	afx_msg void OnBnClickedDeleteSelected();
	afx_msg void OnThumbnailSizeChange();
};