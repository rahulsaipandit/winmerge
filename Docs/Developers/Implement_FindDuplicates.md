# Find Duplicates Feature Implementation

## Overview
The Find Duplicates feature is implemented as a new tabbed interface integrated into the main window's tab system, accessible from the Tools menu in the folder comparison view. It allows users to scan for duplicate files between two folders and manage them safely, using the existing folder paths from the active comparison.

## Key Files and Components

### Core Tab Implementation
- **DuplicateDoc.h/.cpp**: Document class for the duplicate review tab.
- **DuplicateView.h/.cpp**: Form view class that implements the duplicate review UI. Handles folder selection, comparison criteria, ignore patterns, tree population, and duplicate deletion operations.
- **Merge.rc (IDD_DUPLICATE_REVIEW)**: Dialog resource definition adapted for form view containing all UI controls for the duplicate review tab.

### Legacy Dialog (Deprecated)
- **DuplicateReviewDialog.h/.cpp**: Original modal dialog implementation, kept for reference.

### Menu Integration
- **resource.h**: Defines IDB_TOOLS_FIND_DUPLICATES bitmap resource ID
- **Merge2.rc**: Defines the bitmap resource using "res\\search.bmp" for the menu icon
- **MainFrm.cpp**: Maps the ID_TOOLS_FIND_DUPLICATES command to its bitmap icon in the menu system
- **Merge.rc (IDR_DIRDOCTYPE)**: Contains the "Find &Duplicates" menu item in the Tools menu for folder comparison view
- **DirView.h/.cpp**: Contains the OnToolsFindDuplicates command handler that launches the duplicate review dialog

### Supporting Components
- **FileInfo structure**: Used to store file metadata including hash values for duplicate detection
- **PropertySystem**: Manages extended file properties including SHA256 hashes for content comparison
- **FileActionScript**: Contains RemoveDuplicates method for batch deletion operations

## Implementation Flow

1. **Menu Access**: User selects "Tools > Find Duplicates" from folder comparison view
2. **Tab Creation**: DirView::OnToolsFindDuplicates() creates a new tab with CDuplicateView using CMergeApp::GetDuplicateTemplate()
3. **Folder Initialization**: Folders are automatically set from the current folder comparison context
4. **Comparison Setup**: User configures comparison criteria (name, size, timestamp, content) and ignore patterns
5. **Scanning**: PerformComparison() method scans both folders recursively, applying filters and computing hashes
6. **Tree Population**: Results displayed in dual tree views with duplicate groups highlighted
7. **Management**: User can select duplicates for deletion, which moves them to recycle bin

## Key Features
- Integrated tabbed interface with native scrolling and resizing
- Automatic folder path initialization from active comparison
- Recursive folder scanning with ignore pattern filtering
- Multiple comparison criteria with hash-based content verification
- Safe deletion to recycle bin with confirmation dialogs
- Progress tracking and statistics display
- Thumbnail support for media files
- Batch operations for efficient duplicate management