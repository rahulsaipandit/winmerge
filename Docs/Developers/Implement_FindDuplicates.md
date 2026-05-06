# Find Duplicates Feature Implementation

## Overview
The Find Duplicates feature is implemented as a new dialog-based tool accessible from the Tools menu in the folder comparison view. It allows users to scan for duplicate files between two folders and manage them safely.

## Key Files and Components

### Core Dialog Implementation
- **DuplicateReviewDialog.h/.cpp**: Main dialog class that implements the duplicate review UI. Handles folder selection, comparison criteria, ignore patterns, tree population, and duplicate deletion operations.
- **Merge.rc (IDD_DUPLICATE_REVIEW)**: Dialog resource definition containing all UI controls for the duplicate review dialog.

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
2. **Dialog Launch**: DirView::OnToolsFindDuplicates() creates and shows CDuplicateReviewDialog
3. **Folder Selection**: User specifies master (read-only) and duplicate (editable) folders
4. **Comparison Setup**: User configures comparison criteria (name, size, timestamp, content) and ignore patterns
5. **Scanning**: PerformComparison() method scans both folders recursively, applying filters and computing hashes
6. **Tree Population**: Results displayed in dual tree views with duplicate groups highlighted
7. **Management**: User can select duplicates for deletion, which moves them to recycle bin

## Key Features
- Recursive folder scanning with ignore pattern filtering
- Multiple comparison criteria with hash-based content verification
- Safe deletion to recycle bin with confirmation dialogs
- Progress tracking and statistics display
- Thumbnail support for media files
- Batch operations for efficient duplicate management