#include "stdafx.h"
#include "DuplicateView.h"
#include "DuplicateDoc.h"
#include "FileInfo.h"
#include "DirView.h"
#include "MainFrm.h"
#include "FileActionScript.h"
#include "PropertySystem.h"
#include "FileInfo.h"
#include "7zCommon.h"
#include <vector>
#include <memory>
#include <algorithm>

// CDuplicateView

IMPLEMENT_DYNCREATE(CDuplicateView, CFormView)

CDuplicateView::CDuplicateView()
	: CFormView(IDD)
{
}

CDuplicateView::~CDuplicateView()
{
}

void CDuplicateView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GROUPS, m_groups);
	DDX_Control(pDX, IDC_DUPLICATES, m_duplicates);
	DDX_Control(pDX, IDC_RECLAIM, m_reclaim);
	DDX_Control(pDX, IDC_MASTER_FOLDER, m_masterFolder);
	DDX_Control(pDX, IDC_DUPLICATE_FOLDER, m_duplicateFolder);
	DDX_Control(pDX, IDC_NAME_COMPARE, m_nameCompare);
	DDX_Control(pDX, IDC_SIZE_COMPARE, m_sizeCompare);
	DDX_Control(pDX, IDC_TIMESTAMP_COMPARE, m_timestampCompare);
	DDX_Control(pDX, IDC_CONTENT_COMPARE, m_contentCompare);
	DDX_Control(pDX, IDC_IGNORE_PATTERN, m_ignorePattern);
	DDX_Control(pDX, IDC_IGNORE_LIST, m_ignoreList);
	DDX_Control(pDX, IDC_MASTER_TREE, m_masterTree);
	DDX_Control(pDX, IDC_DUPLICATE_TREE, m_duplicateTree);
	DDX_Control(pDX, IDC_THUMBNAIL_SIZE, m_thumbnailSize);
}

BEGIN_MESSAGE_MAP(CDuplicateView, CFormView)
	ON_BN_CLICKED(IDC_REFRESH, &CDuplicateView::OnBnClickedRefresh)
	ON_BN_CLICKED(IDC_BROWSE_MASTER, &CDuplicateView::OnBnClickedBrowseMaster)
	ON_BN_CLICKED(IDC_BROWSE_DUPLICATE, &CDuplicateView::OnBnClickedBrowseDuplicate)
	ON_BN_CLICKED(IDC_ADD_PATTERN, &CDuplicateView::OnBnClickedAddPattern)
	ON_BN_CLICKED(IDC_REMOVE_PATTERN, &CDuplicateView::OnBnClickedRemovePattern)
	ON_BN_CLICKED(IDC_COMPARE, &CDuplicateView::OnBnClickedCompare)
	ON_BN_CLICKED(IDC_DELETE_SELECTED, &CDuplicateView::OnBnClickedDeleteSelected)
	ON_CBN_SELCHANGE(IDC_THUMBNAIL_SIZE, &CDuplicateView::OnThumbnailSizeChange)
END_MESSAGE_MAP()

// CDuplicateView diagnostics

CDuplicateDoc* CDuplicateView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDuplicateDoc)));
	return (CDuplicateDoc*)m_pDocument;
}

#ifdef _DEBUG
void CDuplicateView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CDuplicateView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

// CDuplicateView message handlers

void CDuplicateView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// Initialize controls
	if (!m_initialMaster.IsEmpty())
		m_masterFolder.SetWindowText(m_initialMaster);
	if (!m_initialDuplicate.IsEmpty())
		m_duplicateFolder.SetWindowText(m_initialDuplicate);

	m_nameCompare.SetCheck(BST_CHECKED);
	m_sizeCompare.SetCheck(BST_CHECKED);
	m_timestampCompare.SetCheck(BST_UNCHECKED);
	m_contentCompare.SetCheck(BST_UNCHECKED);

	m_ignoreList.AddString(_T("*.pyc"));
	m_ignoreList.AddString(_T("__pycache__"));
	m_ignoreList.AddString(_T(".git"));
	m_ignoreList.AddString(_T(".svn"));
	m_ignoreList.AddString(_T(".DS_Store"));
	m_ignoreList.AddString(_T("Thumbs.db"));
	m_ignoreList.AddString(_T("node_modules"));

	m_thumbnailSize.AddString(_T("Small"));
	m_thumbnailSize.AddString(_T("Medium"));
	m_thumbnailSize.AddString(_T("Large"));
	m_thumbnailSize.AddString(_T("X-Large"));
	m_thumbnailSize.AddString(_T("XX-Large"));
	m_thumbnailSize.SetCurSel(1); // Medium default

	// Initialize image list for trees
	const int thumbSize = GetThumbnailSize(m_thumbnailSize.GetCurSel());
	CImageList imageList;
	imageList.Create(thumbSize, thumbSize, ILC_COLOR32, 10, 10);
	m_masterTree.SetImageList(&imageList, TVSIL_NORMAL);
	m_duplicateTree.SetImageList(&imageList, TVSIL_NORMAL);

	UpdateStatistics(0, 0, 0);
}

BOOL CDuplicateView::OnInitDialog()
{
	// For form view, OnInitialUpdate is called instead
	return TRUE;
}

void CDuplicateView::SetInitialFolders(const CString& master, const CString& duplicate)
{
	m_initialMaster = master;
	m_initialDuplicate = duplicate;
}

void CDuplicateView::UpdateStatistics(int groups, int duplicates, uint64_t reclaimBytes)
{
	CString str;
	str.Format(_T("Groups: %d"), groups);
	m_groups.SetWindowText(str);
	str.Format(_T("Duplicates: %d"), duplicates);
	m_duplicates.SetWindowText(str);
	str.Format(_T("Reclaim: %llu MB"), reclaimBytes / (1024 * 1024));
	m_reclaim.SetWindowText(str);
}

void CDuplicateView::RepopulateTrees()
{
	// TODO: Implement tree population logic from dialog
}

void CDuplicateView::PerformComparison()
{
	// TODO: Implement comparison logic from dialog
}

void CDuplicateView::OnBnClickedRefresh()
{
	// TODO: Implement
}

void CDuplicateView::OnBnClickedBrowseMaster()
{
	// TODO: Implement browse logic
}

void CDuplicateView::OnBnClickedBrowseDuplicate()
{
	// TODO: Implement browse logic
}

void CDuplicateView::OnBnClickedAddPattern()
{
	// TODO: Implement
}

void CDuplicateView::OnBnClickedRemovePattern()
{
	// TODO: Implement
}

void CDuplicateView::OnBnClickedCompare()
{
	PerformComparison();
}

void CDuplicateView::OnBnClickedDeleteSelected()
{
	// TODO: Implement deletion logic
}

void CDuplicateView::OnThumbnailSizeChange()
{
	// TODO: Implement
}