// +build windows

package file_system

import (
	"os"

	"github.com/skippyr/graffiti"
)

func RevealDirectory(directoryPath *string) {
	entries, err := os.ReadDir(*directoryPath)
	if err != nil {
		throwRevealDirectoryError()
	}
	graffiti.Println("    Size       Kind  Name")
	for _, entry := range entries {
		info, err := entry.Info()
		if err != nil {
			continue
		}
		var sizeInBytes int64
		if info.Mode().IsRegular() {
			sizeInBytes = info.Size()
		}
		mode := uint(info.Mode())
		name := graffiti.EscapePrefixCharacters(info.Name())
		kind := stringifyKind(mode)
		size := stringifySize(sizeInBytes)
		graffiti.Println("%s  %9s  %s", size, kind, name)
	}
	graffiti.Println("")
	graffiti.Println("Path: %s.", graffiti.EscapePrefixCharacters(*directoryPath))
	graffiti.Println("Total: %d entries.", len(entries))
}

