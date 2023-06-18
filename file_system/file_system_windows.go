// +build windows

package file_system

import (
	"os"
	"path/filepath"

	"github.com/skippyr/graffiti"
)

func RevealDirectory(directoryPath *string) {
	entries, err := os.ReadDir(*directoryPath)
	if err != nil {
		throwRevealDirectoryError()
	}
	var quantityOfEntries int
	graffiti.Println("    Size       Kind  Name")
	for _, entry := range entries {
		var mode uint
		var sizeInBytes int64
		name := graffiti.EscapePrefixCharacters(entry.Name())
		entryPath := filepath.Join(*directoryPath, entry.Name())
		resolvedEntryPath, err := filepath.EvalSymlinks(entryPath)
		if err != nil {
			resolvedEntryPath = entryPath
		}
		info, err := os.Stat(resolvedEntryPath)
		if err == nil {
			mode = uint(info.Mode())
			if info.Mode().IsRegular() {
				sizeInBytes = info.Size()
			}
		}
		kind := stringifyKind(mode)
		size := stringifySize(sizeInBytes)
		graffiti.Println("%s  %9s  %s%s", size, kind, name, stringifySymlinkOriginPath(&entryPath))
		quantityOfEntries ++
	}
	graffiti.Println("")
	graffiti.Println("Path: %s.", graffiti.EscapePrefixCharacters(*directoryPath))
	graffiti.Println("Total: %d entries.", quantityOfEntries)
}

