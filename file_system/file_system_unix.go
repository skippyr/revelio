// +build !windows

package file_system

import (
	"os"
	"os/user"
	"fmt"
	"syscall"
	"path/filepath"

	"github.com/skippyr/graffiti"
)

type EntryPermission struct {
	bit       uint
	character rune
	color     string
}

func stringifyPermissions(mode uint) string {
	lackPermissionCharacter := '-'
	multipliers := []uint{
		0100, // User
		010,  // Group
		01,   // Others
	}
	permissions := []EntryPermission{
		{
			// Read
			bit:       04,
			character: 'r',
			color:     "green",
		},
		{
			// Write
			bit:       02,
			character: 'w',
			color:     "yellow",
		},
		{
			// Execute
			bit:       01,
			character: 'x',
			color:     "red",
		},
	}
	var permissionsAsString string
	var octalSum uint
	for _, multiplier := range multipliers {
		for _, permission := range permissions {
			permissionValue := permission.bit * multiplier
			if mode&permissionValue != 0 {
				permissionsAsString += fmt.Sprintf("@F{%s}%c@r", permission.color, permission.character)
				octalSum += permissionValue
			} else {
				permissionsAsString += string(lackPermissionCharacter)
			}
		}
	}
	permissionsAsString += fmt.Sprintf(" (%3o)", octalSum)
	return permissionsAsString
}

func RevealDirectory(directoryPath *string) {
	entries, err := os.ReadDir(*directoryPath)
	if err != nil {
		throwRevealDirectoryError()
	}
	var quantityOfEntries int
	graffiti.Println("     Owner      Size      Permissions       Kind  Name")
	for _, entry := range entries {
		var mode uint
		var sizeInBytes int64
		owner := "-"
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
			systemData := info.Sys()
			if systemData != nil {
				stat := systemData.(*syscall.Stat_t)
				user, err := user.LookupId(fmt.Sprint(stat.Uid))
				if err == nil {
					owner = user.Username
				}
			}
		}
		kind := stringifyKind(mode)
		size := stringifySize(sizeInBytes)
		permissions := stringifyPermissions(mode)
		graffiti.Println("%10s  %s  %s  %9s  %s%s", owner, size, permissions, kind, name, stringifySymlinkOriginPath(&entryPath))
		quantityOfEntries ++
	}
	graffiti.Println("")
	graffiti.Println("Path: %s.", graffiti.EscapePrefixCharacters(*directoryPath))
	graffiti.Println("Total: %d entries.", quantityOfEntries)
}

