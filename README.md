# AUTOSTASH - Enhanced Version

## 🆕 What's New

### Major Features Added:

1. **File Backup Support** 
   - Users can now add both files AND folders to backup
   - Files are copied directly, folders are compressed as .zip

2. **Terminal Status Monitoring**
   - System checks if the secondary log terminal is running
   - Status displayed in main menu with color indicators (🟢 Running / 🔴 Not Running)
   - Terminal automatically restarts when needed for any user action

3. **Enhanced Item Management**
   - Items sorted by type (folders first, then files)
   - Visual indicators: 📁 for folders (green), 📄 for files (white)
   - Separate display sections in settings view
   - Clear indices for easy removal

4. **Improved Sound System**
   - Fixed sound overlap issue
   - Added delay between completion and idle sounds (300ms + 1 second)
   - Different sound for idle state (bell.oga instead of message)
   - Suppressed error output with `2>/dev/null`

5. **Better User Experience**
   - Consistent color coding across all outputs
   - Green for folders, White for files, Orange for indices
   - Terminal status check before every command
   - More informative logging

---

## 📋 Menu Options

```
1. Start Background Backup Cycle - Starts automated recurring backups
2. Add File/Folder to Backup      - Browse and add items to backup list
3. Remove Item from Backup         - View and remove items by index
4. Show Settings                   - Display all configuration and items
5. Change Interval                 - Modify backup frequency
7. Stop Background Backup          - Stop the automatic cycle
8. Change Current Directory        - Navigate filesystem
9. Backup Only Once (One-Time)     - Run single backup cycle
0. Exit Application                - Clean exit
```

---

## 🎨 Color Coding

| Element | Color | Usage |
|---------|-------|-------|
| 📁 Folders | Green | Directories in all lists |
| 📄 Files | White | Files in all lists |
| Indices | Orange | [0], [1], [2]... |
| Success | Green | Completion messages |
| Errors | Red | Error messages |
| Warnings | Yellow | Warnings and notifications |
| Info | Blue | Headers and titles |
| Navigation | Cyan | Directory changes |

---

## 🔧 Technical Changes

### config.h
- Changed `MAX_FOLDERS` to `MAX_ITEMS` (increased to 20)
- Added `ItemType` enum (ITEM_FOLDER, ITEM_FILE)
- Changed global array names: `folders[]` → `items[]`
- Added `item_types[]` array to track types
- Added `terminal_running` flag

### ui.c
- New function: `check_terminal_status()` - Monitors terminal state
- Modified: `add_item()` - Handles both files and folders
- Modified: `remove_item()` - Shows item types before removal
- Modified: `show_settings()` - Separate sections for folders/files
- Enhanced: `get_items_in_directory()` - Returns items with types, sorted
- Updated: `play_sound()` - Better sound selection

### scheduler.c
- Modified: `backup_thread()` - Handles both files and folders differently
- Added delays to prevent sound overlap
- Better timing for sound feedback

### copy_engine.c
- New function: `copy_file()` - Copies individual files
- Existing: `compress_folder()` - Compresses folders
- Added error suppression (`2>/dev/null`)

---

## 🚀 Compilation & Execution

```bash
# Compile
make clean
make

# Run
./autostash
```

---

## 📊 How It Works

### Backup Process:

1. **Folders**: 
   - Compressed using `zip -rq`
   - Saved as `folder_name.zip`
   - Excludes nested `backups/` folders

2. **Files**: 
   - Copied directly using `cp`
   - Preserves original filename
   - No compression applied

### Directory Structure:
```
~/backups/
  └── 2026-01-08_14-30-00/
      ├── Documents.zip      (folder → compressed)
      ├── report.pdf         (file → copied)
      └── Photos.zip         (folder → compressed)
```

---

## 🎵 Sound Feedback

| Event | Sound File | When It Plays |
|-------|-----------|---------------|
| Cycle Start | message-new-instant.oga | Backup begins |
| Cycle Complete | complete.oga | All items backed up |
| Idle/Wait | bell.oga | Before waiting for next cycle |

**Timing**: 300ms gap between completion and idle sounds to prevent overlap.

---

## 🐛 Bug Fixes

1. **Terminal not restarting**: Now calls `ensure_log_terminal()` before every action
2. **Sound overlap**: Added delays and changed idle sound
3. **Unclear removal**: Now shows all items with types before asking for index
4. **Mixed sorting**: Items now properly sorted (folders first, then files)

---

## 💡 Usage Tips

1. **Adding Items**:
   - Use browse mode to see visual indicators (📁/📄)
   - Press `m` for manual path entry
   - Navigate pages with `n`/`b`

2. **Removing Items**:
   - Check current items in Settings (Option 4) first
   - Note the exact index number
   - Use Remove (Option 3) with that index

3. **Terminal Status**:
   - Green dot = Terminal running normally
   - Red dot = Terminal closed (will restart automatically)

4. **Best Practices**:
   - Add important files individually for quick access
   - Add entire folders for bulk backup
   - Check settings regularly to verify backup list

---

## 📝 Notes

- Maximum items: **20** (increased from 10 folders)
- Supports spaces in filenames (paths are quoted)
- Automatic exclusion of `backups/*` to prevent recursion
- Thread-safe operations with mutex locks
- Real-time logging to secondary terminal

---

## 🔮 Future Enhancements (Potential)

- Incremental backups (only changed files)
- Compression options for files
- Backup scheduling (specific times)
- Email notifications on completion
- Web interface for remote monitoring
- Backup integrity verification

---

**Version**: 2.0  
**Last Updated**: January 2026  
**Platform**: Ubuntu/Linux (POSIX-compliant)