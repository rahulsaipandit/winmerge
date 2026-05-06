# WinMerge Developer Documentation

This folder contains WinMerge's developer-documentation.

## Developer Documentation

Developer documentation is in meant for people who want to compile WinMerge themselves, want to know how things work in WinMerge and possibly help with WinMerge development.

## How to compile WinMerge?

To compile WinMerge from source:
1. **Prerequisites**: Visual Studio 2019+ with C++ workload
2. Clone or download the WinMerge source from https://github.com/WinMerge/winmerge
3. Open `Src/WinMerge.vs2022.sln` in Visual Studio
4. Build the solution in Release or Debug mode

### Testing the Find Duplicates Feature
1. **Launch WinMerge**: Run the built `WinMerge.exe` from the output directory
2. **Open folders**: File > Open > Select Folders to compare two directories
3. **Access the feature**: Once the folder compare loads, go to **Tools > Find Duplicates**
4. **Configure the dialog**:
   - Folders are pre-filled from the current comparison
   - Select comparison criteria (name/size enabled by default)
   - Add ignore patterns if needed (defaults are pre-loaded)
   - Choose thumbnail size (Small to XX-Large)
5. **Scan for duplicates**: Click **Compare** to scan
6. **Review results**:
   - View hierarchical trees (left: master/keep, right: duplicates/edit)
   - Check statistics for groups/duplicates/reclaimable space
   - Thumbnails display for image files
7. **Manage duplicates**:
   - Select items in the right tree
   - Click **Delete Selected** to move to Recycle Bin (with confirmation)
8. **Verify**: Ensure thumbnails, hierarchy, and deletion work correctly

**Troubleshooting**: If build issues occur, verify dependencies (e.g., Poco, GDI+) are linked properly and check Visual Studio output for errors. The feature integrates with existing WinMerge infrastructure.

### Important files:

 * [ReadMe for developers](readme-developers.html) is old "main" document. Contains still lots and lots of useful information.
 * [Translation instructions](../../Translations/README.md)
 * [User manual information](../Users/Manual/README.md) tells how to convert manual from DocBook to HTML and HTML help.
 * [Options](Options.html) explains WinMerge's options handling.
 * [Plugins](../../Plugins/README.md) contains documentation about plugins-system.
 * [ShellExtension](../../ShellExtension/README.md) contains tipps for the Windows Explorer shell integration.
 * [InnoSetup](../../Installer/InnoSetup/README.md) contains documentation about WinMerge's installer.
 * [Unit testing](UnitTesting.html) instructions for unit testing in WinMerge development.