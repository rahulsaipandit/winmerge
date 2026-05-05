# Add Duplicate Detection Feature Documentation

## Overview
WinMerge is a great Folder Comparison Tool and a graphical application designed to compare the contents of two directories or files but it does not have a duplicate detection feature.

We want to extend this functionality to enable detecting, reviewing, and managing duplicate files especially for photos. 
- Allow users to filter out unwanted file types and items with customizable ignore patterns, and safely manage duplicate or unnecessary files by moving them to the recycle bin. 
- Able to handle exact hash-based duplicate detection, filtering, selection, and deletion operations. 
- Make UI more friendly for media files and for cleaning up backups, synchronizing folders, or identifying redundant data across directories. An example would be to make it easy to do a visual inspection of photo files by showing them as thumbnail previews. 
 
The application leverages existing WinMerge logic to ensuring a responsive experience even with large directory structures. It supports recursive scanning, meaning it delves into subfolders to provide a complete overview of both source and target locations.

## User Interface Overview
We introduce a new menu option called Find Duplicates, that will show a new UI cloned from existing WinMerge UI but modified to meet our requirements. 

### Header Section
- **Title**: "Duplicate Review" 
- **Refresh Button**: Triggers new duplicate scan
- **Close Button**: Emits close event

### Statistics Cards (3-column grid)
- **Groups**: Total number of duplicate groups
- **Duplicates**: Total number of duplicate files
- **Reclaim**: Total reclaimable space from duplicates

### Loading State - Leverage same indicator that WinMerge already uses

### Main Content Area (Grid Layout: Left and Right Split Panes like WinMerge already has)
The main content area is a spacious main window providing ample room to view complex folder hierarchies without clutter. The layout is organized into logical sections, flowing from top to bottom: comparison settings, ignore options, folder selection, visual trees, and action controls.

Key features:
- Real-time duplicate scanning with progress tracking
- Media type filtering (All, Image, RAW, Video)
- Target folder specific duplicate selection and deletion
- Master Folder - Keep/protect mechanism for files
- Batch deletion with reclaimable space calculation
- Folder comparison: Compare two arbitrary directories (source vs. target) instead of library-based scanning
- Custom ignore patterns with wildcard support and pre-loaded defaults
- Multiple comparison criteria (name, size, timestamp with tolerance, binary content)
- Dual tree views for hierarchical folder comparison
- Manual folder selection via browse dialogs
- Target-specific deletion with confirmation and recycle bin usage

### Comparison Criteria Section
At the top sits a neatly grouped box labeled "Comparison Criteria," where you can fine-tune how the tool determines file similarity. This section uses a grid layout with checkboxes, each representing a different aspect of file comparison:

- **Name Comparison**: By default, this is enabled, ensuring that files are only considered similar if they share the exact same filename. This is fundamental for most comparisons, as it establishes the baseline match before other criteria are applied.
- **Size Comparison**: Also enabled by default, this checks whether files occupy the same amount of disk space in bytes. It's a quick way to filter out obviously different files without diving into content.
- **Timestamp Comparison**: When selected, the tool compares the last modified dates of files, allowing for a small tolerance of up to one second to account for minor timing discrepancies during file operations or transfers.
- **Content Comparison**: For the most thorough check, enable this to perform a binary comparison of file contents. This is resource-intensive for large files, so use it selectively when size and name alone aren't sufficient.

You can mix and match these options to suit your needs—perhaps focusing only on names and sizes for a quick scan, or enabling all for a comprehensive audit. At least one criterion must be active for the comparison to proceed, preventing empty results.

### Ignore Patterns Section
Directly below the comparison settings is the "Ignore Patterns" group, a flexible feature for excluding files and folders you don't want cluttering your comparison. This section empowers you to customize what's hidden from view, streamlining your focus on relevant items.

A horizontal input bar invites you to enter new wildcard patterns, with a placeholder text guiding you: "Enter wildcard pattern (e.g., *.txt, temp*, .git)". Once typed, a simple "Add" button appends it to your personal ignore list. The tool comes pre-loaded with sensible defaults—common exclusions like Python cache files (*.pyc, __pycache__), version control directories (.git, .svn), system files (.DS_Store, Thumbs.db), and bulky folders (node_modules)—saving you from manually entering them each time.

Beneath the input, a list widget displays all active patterns in a scrollable view, each as a selectable item. If a pattern no longer serves your purpose, highlight it and click "Remove Selected" to delete it instantly. The ignore logic is smart: it checks not just filenames, but also parent directory names against your patterns, ensuring entire branches of unwanted data are filtered out during scans.

### File / Folder Selection Area
This works similar to how WinMerge works, with a source and target folder or files. No change there.

### Tree Views for Comparison
The heart of the interface lies in the dual tree views, side-by-side panels that mirror the hierarchical structure of your selected folders. Each tree is labeled clearly: "Read Only Master Folder (Keep)" on the left and "Duplicate Folder (Edit)" on the right.

As you compare, the source tree on the left shows all qualifying items from your first directory, while the target tree on the right displays the second. Similar files—those matching your selected criteria—automatically have their "Similar" checkboxes ticked, visually highlighting potential duplicates or matches. You can manually check or uncheck these to select specific items for action, providing granular control over what to keep or remove.

The trees support full expansion and collapse, letting you navigate deeply nested structures without losing context. Ignored items are seamlessly filtered out, keeping the view focused and performant.

### Action Buttons
At the bottom, two prominent buttons drive the tool's functionality:

- **Compare Button**: This initiates the scanning process, populating both trees with the latest data based on your criteria and ignore settings. It's only usable after selecting both folders, with a warning dialog preventing premature attempts.
- **Delete Selected Button**: Initially disabled until a comparison completes, this becomes active for targeted cleanup. When clicked, it prompts a confirmation dialog—"Are you sure you want to move checked items from the target folder to the recycle bin?"—to avoid accidental data loss. Upon approval, selected items from the target tree are safely moved to the recycle bin using system-level operations, not permanently erased. The view refreshes automatically afterward, reflecting the changes.

## Functionality in Detail
Beyond the UI, the tool's core behavior ensures reliable, safe operations. Folder comparisons recurse through subdirectories, building complete trees while respecting your ignore filters. File similarities are evaluated progressively: starting with existence checks, then applying each enabled criterion in sequence for efficiency.

Deletion is handled with care—items are sent to the recycle bin, preserving recovery options, and any errors (like permission issues) are surfaced in clear message boxes. The application maintains state across operations, such as keeping folder selections intact during refreshes.

Designed for Windows, it leverages OS-specific features like native dialogs and the recycle bin, while remaining portable to other platforms with equivalent libraries. Whether you're a developer cleaning up build artifacts, a photographer organizing archives, or an admin syncing backups, this tool adapts to your workflow with descriptive, user-friendly controls.

## Internal Logic and Algorithms
To replicate the tool's behavior accurately, the underlying logic operates through several interconnected processes, ensuring consistency and predictability.

### Ignore Pattern Filtering
When scanning directories, the tool applies ignore patterns to exclude irrelevant files and folders. For each item encountered, it iterates through the list of patterns (including defaults and user-added ones). A match occurs if the filename directly fits a wildcard pattern or if any segment of the relative path matches, allowing entire directory branches to be skipped. This prevents clutter in the trees and speeds up comparisons by avoiding unnecessary processing of common exclusions like cache or version control directories.

### File Similarity Assessment
Similarity checking is criterion-based and incremental. First, both files must exist on disk. If name comparison is enabled, their basenames must match exactly. For size, file sizes in bytes are compared for equality. Timestamp checks allow a one-second difference in modification times to handle minor variations. Content comparison uses binary file comparison when selected, which can be slow for large files but ensures byte-for-byte equivalence. Only if all enabled criteria pass does a file qualify as similar, with the process short-circuiting early if any fail.

### Tree Population Mechanism
Populating the tree views involves recursive directory traversal using the system's walk function. For each directory level, subdirectories are filtered based on ignore patterns before descending. Files and folders are added as tree items, with folders showing no size and files displaying size and modification time in a formatted string. If a comparison path is provided (during folder comparison), files are checked for similarity to their counterparts, automatically ticking checkboxes for matches. This creates a hierarchical representation that mirrors the filesystem, with ignored items completely absent.

### Folder Comparison Process
The comparison workflow begins with validation that both source and target paths are set. It then populates the source tree with items from the source directory, comparing each to the target, and similarly for the target tree. This bidirectional check ensures symmetry, though actions like deletion target only the specified tree. The process is efficient, leveraging the filtering to reduce the dataset early.

### Deletion Handling
Deletion targets checked items in the target tree, traversing the tree structure recursively to build full file paths. For each checked item, it attempts to move the file or directory to the recycle bin using system-safe operations. Errors are collected and displayed collectively, preventing partial failures from halting the process. After completion, the trees refresh to reflect the updated state, maintaining the comparison view's integrity.