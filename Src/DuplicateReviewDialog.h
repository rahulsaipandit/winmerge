// DuplicateReviewDialog.h
#pragma once

#include <afxwin.h>
#include <afxcmn.h>
#include "resource.h"
#include <memory>

// Defined in DuplicateReviewDialog.cpp — keeps FileInfo out of this header.
struct CDuplicateReviewDialogCache;

class CDuplicateReviewDialog : public CDialog
{
    DECLARE_DYNAMIC(CDuplicateReviewDialog)

public:
    CDuplicateReviewDialog(CWnd* pParent = nullptr);
    virtual ~CDuplicateReviewDialog();

    void SetInitialFolders(const CString& master, const CString& duplicate);

    enum { IDD = IDD_DUPLICATE_REVIEW };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedRefresh();
    afx_msg void OnBnClickedBrowseMaster();
    afx_msg void OnBnClickedBrowseDuplicate();
    afx_msg void OnBnClickedAddPattern();
    afx_msg void OnBnClickedRemovePattern();
    afx_msg void OnBnClickedCompare();
    afx_msg void OnBnClickedDeleteSelected();
    afx_msg void OnThumbnailSizeChange();

    DECLARE_MESSAGE_MAP()

private:
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
    CStatic m_groupsLabel;
    CStatic m_duplicatesLabel;
    CStatic m_reclaimLabel;
    CComboBox m_thumbnailSize;
    CImageList m_imageList;
    ULONG_PTR m_gdiplusToken = 0;
    CString m_initialMaster;
    CString m_initialDuplicate;

    // Caches scan results so thumbnail-size changes don't trigger a re-scan.
    std::unique_ptr<CDuplicateReviewDialogCache> m_cache;

    void UpdateStatistics(int groups, int duplicates, uint64_t reclaimBytes);
    void RepopulateTrees();
    void PerformComparison();
};
