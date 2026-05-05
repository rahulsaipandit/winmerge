// DuplicateReviewDialog.cpp
#include "stdafx.h"
#include "DuplicateReviewDialog.h"
#include "PropertySystem.h"
#include "DirItem.h"
#include "unicoder.h"
#include <shlobj.h>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <algorithm>
#include <gdiplus.h>
#include <map>
#include <set>
#include <vector>

struct FileInfo
{
	String path;
	uint64_t size = 0;
	FILETIME mtime = {0, 0};
	std::vector<uint8_t> hash;
	bool isInMaster = false;
};

struct CDuplicateReviewDialogCache
{
	std::vector<FileInfo> masterFiles;
	std::vector<FileInfo> duplicateFiles;
	std::set<String> duplicatePaths;
	int totalGroups = 0;
	int totalDuplicates = 0;
	uint64_t reclaimBytes = 0;
};

// ---------------------------------------------------------------------------
// Anonymous-namespace helpers
// ---------------------------------------------------------------------------
namespace
{
constexpr const tchar_t* DuplicateHashProperty = _T("Hash.SHA256");

bool MatchesIgnorePattern(const String& path, const std::vector<String>& patterns)
{
	for (const auto& pattern : patterns)
	{
		if (PathMatchSpec(path.c_str(), pattern.c_str()))
			return true;
	}
	return false;
}

void ScanFolderRecursive(const String& folder, const std::vector<String>& ignorePatterns, std::vector<FileInfo>& files)
{
	try
	{
		Poco::File dir(folder.c_str());
		if (!dir.exists() || !dir.isDirectory())
			return;

		std::vector<Poco::File> contents;
		dir.list(contents);

		for (const auto& file : contents)
		{
			String path = file.path().c_str();
			String relPath = path.substr(folder.length() + 1);

			if (MatchesIgnorePattern(relPath, ignorePatterns))
				continue;

			if (file.isDirectory())
			{
				ScanFolderRecursive(path, ignorePatterns, files);
			}
			else
			{
				FileInfo fi;
				fi.path = path;
				fi.size = file.getSize();
				FILETIME ft = {};
				HANDLE hFile = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ,
					nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					GetFileTime(hFile, nullptr, nullptr, &ft);
					CloseHandle(hFile);
				}
				fi.mtime = ft;
				files.push_back(fi);
			}
		}
	}
	catch (...)
	{
	}
}

std::vector<FileInfo> ScanFolder(const String& folder, const std::vector<String>& ignorePatterns)
{
	std::vector<FileInfo> files;
	ScanFolderRecursive(folder, ignorePatterns, files);
	return files;
}

// Takes pointers so it operates on the exact FileInfo objects stored in allFiles.
void ComputeHashes(std::vector<FileInfo*>& files, PropertySystem& propSys)
{
	const int hashIndex = propSys.GetPropertyIndex(DuplicateHashProperty);
	if (hashIndex < 0)
		return;

	for (auto* fi : files)
	{
		if (!fi->hash.empty())
			continue;
		try
		{
			std::unique_ptr<PropertyValues> props(new PropertyValues());
			propSys.GetPropertyValues(fi->path, *props);
			if (props->IsHashValue(hashIndex))
				fi->hash = props->GetHashValue(hashIndex);
		}
		catch (...)
		{
		}
	}
}

String FormatDuplicateBytes(uint64_t bytes)
{
	static constexpr uint64_t KiB = 1024;
	static constexpr uint64_t MiB = KiB * 1024;
	static constexpr uint64_t GiB = MiB * 1024;

	if (bytes >= GiB)
		return strutils::format(_T("%.2f GB"), static_cast<double>(bytes) / GiB);
	if (bytes >= MiB)
		return strutils::format(_T("%.2f MB"), static_cast<double>(bytes) / MiB);
	if (bytes >= KiB)
		return strutils::format(_T("%.2f KB"), static_cast<double>(bytes) / KiB);
	return strutils::format(_T("%I64u bytes"), static_cast<unsigned long long>(bytes));
}

int GetThumbnailSize(int index)
{
	switch (index)
	{
	case 0: return 16;
	case 1: return 32;
	case 2: return 64;
	case 3: return 128;
	case 4: return 256;
	default: return 32;
	}
}

bool IsImageFile(const String& path)
{
	String ext = Poco::Path(path).getExtension();
	ext = strutils::to_lower(ext);
	return ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "bmp"
		|| ext == "gif" || ext == "tiff" || ext == "tif" || ext == "webp";
}

HBITMAP CreateThumbnail(const String& path, int size)
{
	Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(path.c_str());
	if (!bitmap || bitmap->GetLastStatus() != Gdiplus::Ok)
	{
		delete bitmap;
		return nullptr;
	}

	Gdiplus::Bitmap* thumb = static_cast<Gdiplus::Bitmap*>(bitmap->GetThumbnailImage(size, size));
	delete bitmap;

	if (!thumb || thumb->GetLastStatus() != Gdiplus::Ok)
	{
		delete thumb;
		return nullptr;
	}

	HBITMAP hBitmap = nullptr;
	thumb->GetHBITMAP(Gdiplus::Color::White, &hBitmap);
	delete thumb;

	return hBitmap;
}

HTREEITEM InsertTreeItem(CTreeCtrl& tree, HTREEITEM parent, const String& text, LPARAM data = 0)
{
	TVINSERTSTRUCT tvis = {0};
	tvis.hParent = parent;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
	tvis.item.pszText = const_cast<LPTSTR>(text.c_str());
	tvis.item.lParam = data;
	return tree.InsertItem(&tvis);
}

void CollectCheckedItems(CTreeCtrl& tree, HTREEITEM item, std::vector<String>& paths)
{
	while (item)
	{
		if (tree.GetCheck(item))
		{
			FileInfo* fi = reinterpret_cast<FileInfo*>(tree.GetItemData(item));
			if (fi)
				paths.push_back(fi->path);
		}
		// Recurse into children
		HTREEITEM child = tree.GetChildItem(item);
		if (child)
			CollectCheckedItems(tree, child, paths);
		item = tree.GetNextSiblingItem(item);
	}
}

std::vector<String> GetCheckedPaths(CTreeCtrl& tree)
{
	std::vector<String> paths;
	CollectCheckedItems(tree, tree.GetRootItem(), paths);
	return paths;
}

// Populates one tree from a flat file list.  The caller owns the image list
// lifecycle; this function only appends images (never clears the list).
void PopulateTreeFromFiles(CTreeCtrl& tree, CImageList& imageList,
	const std::vector<FileInfo>& files, const String& rootPath,
	bool isMaster, const std::set<String>& duplicatePaths, int thumbSize)
{
	tree.DeleteAllItems();

	// Normalize root: strip trailing backslash so substr arithmetic is uniform.
	String root = rootPath;
	if (!root.empty() && root.back() == _T('\\'))
		root.pop_back();

	std::map<String, HTREEITEM> folderItems;

	for (const auto& fi : files)
	{
		if (fi.path.length() <= root.length() + 1)
			continue;

		String relPath = fi.path.substr(root.length() + 1);

		// Split on backslash into path components.
		std::vector<String> parts;
		String remaining = relPath;
		size_t pos;
		while ((pos = remaining.find(_T('\\'))) != String::npos)
		{
			if (pos > 0)
				parts.push_back(remaining.substr(0, pos));
			remaining = remaining.substr(pos + 1);
		}
		parts.push_back(remaining); // filename is always last

		if (parts.empty())
			continue;

		HTREEITEM parent = TVI_ROOT;
		String currentPath;
		for (size_t i = 0; i + 1 < parts.size(); ++i) // folder components
		{
			currentPath += parts[i] + _T("\\");
			auto it = folderItems.find(currentPath);
			if (it == folderItems.end())
			{
				it = folderItems.emplace(currentPath,
					InsertTreeItem(tree, parent, parts[i])).first;
			}
			parent = it->second;
		}

		String text = strutils::format(_T("%s (%s)"),
			parts.back().c_str(), FormatDuplicateBytes(fi.size).c_str());

		int iconIndex = -1;
		if (IsImageFile(fi.path))
		{
			HBITMAP hThumb = CreateThumbnail(fi.path, thumbSize);
			if (hThumb)
			{
				iconIndex = imageList.Add(CBitmap::FromHandle(hThumb), (CBitmap*)nullptr);
				DeleteObject(hThumb); // CImageList copied the bits; release the GDI object
			}
		}

		HTREEITEM item = InsertTreeItem(tree, parent, text, reinterpret_cast<LPARAM>(&fi));
		if (iconIndex >= 0)
			tree.SetItemImage(item, iconIndex, iconIndex);
		if (!isMaster && duplicatePaths.count(fi.path))
			tree.SetCheck(item, TRUE);
	}
}

void ProcessDuplicateGroup(const std::vector<FileInfo*>& group,
	std::set<String>& duplicatePaths, int& totalDuplicates, uint64_t& reclaimBytes)
{
	std::vector<FileInfo*> masters, duplicates;
	for (auto* fi : group)
	{
		if (fi->isInMaster)
			masters.push_back(fi);
		else
			duplicates.push_back(fi);
	}

	if (duplicates.empty())
		return;

	// If there's a master copy, all duplicate-folder copies are redundant.
	// If there's no master, keep one (the first after sort) and flag the rest.
	const size_t toDelete = duplicates.size() - (masters.empty() ? 1 : 0);
	for (size_t i = 0; i < toDelete; ++i)
	{
		duplicatePaths.insert(duplicates[i]->path);
		++totalDuplicates;
		reclaimBytes += duplicates[i]->size;
	}
}

} // namespace

// ---------------------------------------------------------------------------
// CDuplicateReviewDialog
// ---------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CDuplicateReviewDialog, CDialog)

CDuplicateReviewDialog::CDuplicateReviewDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DUPLICATE_REVIEW, pParent)
	, m_gdiplusToken(0)
{
}

void CDuplicateReviewDialog::SetInitialFolders(const CString& master, const CString& duplicate)
{
	m_initialMaster = master;
	m_initialDuplicate = duplicate;
}

CDuplicateReviewDialog::~CDuplicateReviewDialog()
{
	if (m_gdiplusToken)
		Gdiplus::GdiplusShutdown(m_gdiplusToken);
}

void CDuplicateReviewDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
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
	DDX_Control(pDX, IDC_GROUPS, m_groupsLabel);
	DDX_Control(pDX, IDC_DUPLICATES, m_duplicatesLabel);
	DDX_Control(pDX, IDC_RECLAIM, m_reclaimLabel);
	DDX_Control(pDX, IDC_THUMBNAIL_SIZE, m_thumbnailSize);
}

BEGIN_MESSAGE_MAP(CDuplicateReviewDialog, CDialog)
	ON_BN_CLICKED(IDC_REFRESH, &CDuplicateReviewDialog::OnBnClickedRefresh)
	ON_BN_CLICKED(IDC_BROWSE_MASTER, &CDuplicateReviewDialog::OnBnClickedBrowseMaster)
	ON_BN_CLICKED(IDC_BROWSE_DUPLICATE, &CDuplicateReviewDialog::OnBnClickedBrowseDuplicate)
	ON_BN_CLICKED(IDC_ADD_PATTERN, &CDuplicateReviewDialog::OnBnClickedAddPattern)
	ON_BN_CLICKED(IDC_REMOVE_PATTERN, &CDuplicateReviewDialog::OnBnClickedRemovePattern)
	ON_BN_CLICKED(IDC_COMPARE, &CDuplicateReviewDialog::OnBnClickedCompare)
	ON_BN_CLICKED(IDC_DELETE_SELECTED, &CDuplicateReviewDialog::OnBnClickedDeleteSelected)
	ON_CBN_SELCHANGE(IDC_THUMBNAIL_SIZE, &CDuplicateReviewDialog::OnThumbnailSizeChange)
END_MESSAGE_MAP()

BOOL CDuplicateReviewDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

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
	m_thumbnailSize.SetCurSel(1); // Medium (32 px) default

	Gdiplus::GdiplusStartup(&m_gdiplusToken, &Gdiplus::GdiplusStartupInput(), nullptr);

	const int thumbSize = GetThumbnailSize(m_thumbnailSize.GetCurSel());
	m_imageList.Create(thumbSize, thumbSize, ILC_COLOR32, 10, 10);
	m_masterTree.SetImageList(&m_imageList, TVSIL_NORMAL);
	m_duplicateTree.SetImageList(&m_imageList, TVSIL_NORMAL);

	UpdateStatistics(0, 0, 0);
	return TRUE;
}

void CDuplicateReviewDialog::OnBnClickedRefresh()
{
	PerformComparison();
}

void CDuplicateReviewDialog::OnBnClickedBrowseMaster()
{
	BROWSEINFO bi = {0};
	bi.lpszTitle = _T("Select Master Folder");
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (pidl != nullptr)
	{
		TCHAR path[MAX_PATH];
		if (SHGetPathFromIDList(pidl, path))
			m_masterFolder.SetWindowText(path);
		CoTaskMemFree(pidl);
	}
}

void CDuplicateReviewDialog::OnBnClickedBrowseDuplicate()
{
	BROWSEINFO bi = {0};
	bi.lpszTitle = _T("Select Duplicate Folder");
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (pidl != nullptr)
	{
		TCHAR path[MAX_PATH];
		if (SHGetPathFromIDList(pidl, path))
			m_duplicateFolder.SetWindowText(path);
		CoTaskMemFree(pidl);
	}
}

void CDuplicateReviewDialog::OnBnClickedAddPattern()
{
	CString pattern;
	m_ignorePattern.GetWindowText(pattern);
	if (!pattern.IsEmpty())
	{
		m_ignoreList.AddString(pattern);
		m_ignorePattern.SetWindowText(_T(""));
	}
}

void CDuplicateReviewDialog::OnBnClickedRemovePattern()
{
	const int sel = m_ignoreList.GetCurSel();
	if (sel != LB_ERR)
		m_ignoreList.DeleteString(sel);
}

void CDuplicateReviewDialog::OnBnClickedCompare()
{
	PerformComparison();
}

void CDuplicateReviewDialog::OnBnClickedDeleteSelected()
{
	auto paths = GetCheckedPaths(m_duplicateTree);
	if (paths.empty())
	{
		AfxMessageBox(_T("No items selected for deletion."));
		return;
	}

	if (AfxMessageBox(_T("Are you sure you want to move the selected items to the Recycle Bin?"),
		MB_YESNO | MB_ICONQUESTION) != IDYES)
		return;

	// SHFileOperation requires a double-null-terminated flat list of paths.
	std::vector<TCHAR> fileList;
	for (const auto& path : paths)
	{
		fileList.insert(fileList.end(), path.begin(), path.end());
		fileList.push_back(_T('\0'));
	}
	fileList.push_back(_T('\0'));

	SHFILEOPSTRUCT shfo = {0};
	shfo.wFunc = FO_DELETE;
	shfo.pFrom = fileList.data();
	shfo.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;

	if (SHFileOperation(&shfo) == 0)
	{
		AfxMessageBox(_T("Selected items moved to Recycle Bin."));
		PerformComparison();
	}
	else
	{
		AfxMessageBox(_T("Error moving items to Recycle Bin."));
	}
}

void CDuplicateReviewDialog::OnThumbnailSizeChange()
{
	RepopulateTrees();
}

void CDuplicateReviewDialog::UpdateStatistics(int groups, int duplicates, uint64_t reclaimBytes)
{
	CString str;
	str.Format(_T("Groups: %d"), groups);
	m_groupsLabel.SetWindowText(str);
	str.Format(_T("Duplicates: %d"), duplicates);
	m_duplicatesLabel.SetWindowText(str);
	str.Format(_T("Reclaim: %s"), FormatDuplicateBytes(reclaimBytes).c_str());
	m_reclaimLabel.SetWindowText(str);
}

void CDuplicateReviewDialog::RepopulateTrees()
{
	if (!m_cache)
		return;

	const int thumbSize = GetThumbnailSize(m_thumbnailSize.GetCurSel());

	// Recreate the shared image list at the new dimensions before populating.
	m_imageList.DeleteImageList();
	m_imageList.Create(thumbSize, thumbSize, ILC_COLOR32, 10, 10);
	m_masterTree.SetImageList(&m_imageList, TVSIL_NORMAL);
	m_duplicateTree.SetImageList(&m_imageList, TVSIL_NORMAL);

	CString masterPath, duplicatePath;
	m_masterFolder.GetWindowText(masterPath);
	m_duplicateFolder.GetWindowText(duplicatePath);

	// Both trees share m_imageList: master populates indices 0..N, then
	// duplicate continues from N+1.  Neither call clears the list.
	PopulateTreeFromFiles(m_masterTree, m_imageList,
		m_cache->masterFiles, masterPath.GetString(),
		true, m_cache->duplicatePaths, thumbSize);
	PopulateTreeFromFiles(m_duplicateTree, m_imageList,
		m_cache->duplicateFiles, duplicatePath.GetString(),
		false, m_cache->duplicatePaths, thumbSize);

	UpdateStatistics(m_cache->totalGroups, m_cache->totalDuplicates, m_cache->reclaimBytes);
}

void CDuplicateReviewDialog::PerformComparison()
{
	CString masterPath, duplicatePath;
	m_masterFolder.GetWindowText(masterPath);
	m_duplicateFolder.GetWindowText(duplicatePath);

	if (masterPath.IsEmpty() || duplicatePath.IsEmpty())
	{
		AfxMessageBox(_T("Please select both folders."));
		return;
	}

	std::vector<String> ignorePatterns;
	const int count = m_ignoreList.GetCount();
	for (int i = 0; i < count; ++i)
	{
		CString str;
		m_ignoreList.GetText(i, str);
		ignorePatterns.push_back(str.GetString());
	}

	auto masterFiles = ScanFolder(masterPath.GetString(), ignorePatterns);
	auto duplicateFiles = ScanFolder(duplicatePath.GetString(), ignorePatterns);

	for (auto& fi : masterFiles)   fi.isInMaster = true;
	for (auto& fi : duplicateFiles) fi.isInMaster = false;

	// Combine for grouping, but keep separate vectors for tree population.
	std::vector<FileInfo> allFiles;
	allFiles.insert(allFiles.end(), masterFiles.begin(), masterFiles.end());
	allFiles.insert(allFiles.end(), duplicateFiles.begin(), duplicateFiles.end());

	const bool useName      = m_nameCompare.GetCheck()      == BST_CHECKED;
	const bool useSize      = m_sizeCompare.GetCheck()      == BST_CHECKED;
	const bool useTimestamp = m_timestampCompare.GetCheck() == BST_CHECKED;
	const bool useContent   = m_contentCompare.GetCheck()   == BST_CHECKED;

	if (!useName && !useSize && !useTimestamp && !useContent)
	{
		AfxMessageBox(_T("Please select at least one comparison criterion."));
		return;
	}

	// Primary grouping by (name, size) to avoid hashing unrelated files.
	std::map<std::tuple<String, uint64_t>, std::vector<FileInfo*>> nameSizeGroups;
	for (auto& fi : allFiles)
	{
		String name = Poco::Path(fi.path).getFileName();
		nameSizeGroups[{name, fi.size}].push_back(&fi);
	}

	std::unique_ptr<PropertySystem> propSys;
	if (useContent)
		propSys.reset(new PropertySystem({DuplicateHashProperty}));

	std::set<String> duplicatePaths;
	int totalGroups    = 0;
	int totalDuplicates = 0;
	uint64_t reclaimBytes = 0;

	for (auto& entry : nameSizeGroups)
	{
		auto& group = entry.second;
		if (group.size() < 2)
			continue;

		if (useContent)
			ComputeHashes(group, *propSys);

		std::map<std::vector<uint8_t>, std::vector<FileInfo*>> hashGroups;
		if (useContent)
		{
			for (auto* fi : group)
				hashGroups[fi->hash].push_back(fi);
		}
		else
		{
			hashGroups[{}].assign(group.begin(), group.end());
		}

		for (auto& hentry : hashGroups)
		{
			auto& hgroup = hentry.second;
			if (hgroup.size() < 2)
				continue;

			if (useTimestamp)
			{
				// Sort ascending by modification time, then cluster within 1 second
				// (FILETIME unit = 100 ns; 1 s = 10 000 000 units).
				auto ftToULL = [](const FILETIME& ft) -> uint64_t {
					return (static_cast<uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
				};
				constexpr uint64_t kOneSecond = 10000000ULL;

				std::sort(hgroup.begin(), hgroup.end(), [&ftToULL](FileInfo* a, FileInfo* b) {
					return ftToULL(a->mtime) < ftToULL(b->mtime);
				});

				std::vector<std::vector<FileInfo*>> timeGroups;
				for (auto* fi : hgroup)
				{
					if (timeGroups.empty())
					{
						timeGroups.push_back({fi});
					}
					else
					{
						const uint64_t last = ftToULL(timeGroups.back().back()->mtime);
						const uint64_t cur  = ftToULL(fi->mtime);
						if (cur - last <= kOneSecond)
							timeGroups.back().push_back(fi);
						else
							timeGroups.push_back({fi});
					}
				}
				for (auto& tgroup : timeGroups)
				{
					if (tgroup.size() >= 2)
					{
						ProcessDuplicateGroup(tgroup, duplicatePaths, totalDuplicates, reclaimBytes);
						++totalGroups;
					}
				}
			}
			else
			{
				ProcessDuplicateGroup(hgroup, duplicatePaths, totalDuplicates, reclaimBytes);
				++totalGroups;
			}
		}
	}

	// Cache results so thumbnail-size changes don't trigger a re-scan.
	m_cache.reset(new CDuplicateReviewDialogCache());
	m_cache->masterFiles    = std::move(masterFiles);
	m_cache->duplicateFiles = std::move(duplicateFiles);
	m_cache->duplicatePaths = std::move(duplicatePaths);
	m_cache->totalGroups    = totalGroups;
	m_cache->totalDuplicates = totalDuplicates;
	m_cache->reclaimBytes   = reclaimBytes;

	RepopulateTrees();
}
