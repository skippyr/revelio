// +build !windows

package file_system

import (
	"os"
	"os/user"
	"fmt"
	"syscall"

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
	permissionsAsString += fmt.Sprintf(" (%o)", octalSum)
	return permissionsAsString
}

func RevealDirectory(directoryPath *string) {
	entries, err := os.ReadDir(*directoryPath)
	if err != nil {
		throwRevealDirectoryError()
	}
	graffiti.Println("     Owner      Size      Permissions       Kind   Name")
	for _, entry := range entries {
		info, err := entry.Info()
		systemData := info.Sys()
		if err != nil || systemData == nil {
			continue
		}
		stat := systemData.(*syscall.Stat_t)
		user, err := user.LookupId(fmt.Sprint(stat.Uid))
		if err != nil {
			continue
		}
		var sizeInBytes int64
		if info.Mode().IsRegular() {
			sizeInBytes = info.Size()
		}
		mode := uint(info.Mode())
		name := graffiti.EscapePrefixCharacters(info.Name())
		owner := user.Username
		permission := stringifyPermissions(mode)
		kind := stringifyKind(mode)
		size := stringifySize(sizeInBytes)
		graffiti.Println("%10s  %s  %s  %9s  %s", owner, size, permission, kind, name)
	}
	graffiti.Println("")
	graffiti.Println("Path: %s.", graffiti.EscapePrefixCharacters(*directoryPath))
	graffiti.Println("Total: %d entries.", len(entries))
}

