package file_system

import (
	"io/fs"
	"os"
	"fmt"

	"github.com/skippyr/reveal/errors"
)

type EntryKind struct {
	bit  uint
	name string
}

type DigitalSizeUnit struct {
	bytes      int64
	character  rune
}

func stringifyKind(mode uint) string {
	kinds := []EntryKind{
		{
			bit:  uint(fs.ModeDir),
			name: "Directory",
		},
		{
			bit:  uint(fs.ModeDevice),
			name: "Device",
		},
		{
			bit:  uint(fs.ModeNamedPipe),
			name: "Fifo",
		},
		{
			bit:  uint(fs.ModeSocket),
			name: "Socket",
		},
	}
	for _, kind := range kinds {
		if kind.bit&mode != 0 {
			return kind.name
		}
	}
	return "File"
}

func stringifySize(sizeInBytes int64) string {
	if sizeInBytes == 0 {
		return fmt.Sprintf("%8s", "-")
	}
	units := []DigitalSizeUnit{
		{
			// GigaBytes
			bytes:     1e9,
			character: 'G',
		},
		{
			// MegaBytes
			bytes:     1e6,
			character: 'M',
		},
		{
			// KiloBytes
			bytes:     1e3,
			character: 'K',
		},
	}
	for _, unit := range units {
		unitSize := float32(sizeInBytes) / float32(unit.bytes)
		if int(unitSize) > 0 {
			return fmt.Sprintf("%6.1f%cB", unitSize, unit.character)
		}
	}
	return fmt.Sprintf("%7dB", sizeInBytes)
}

func stringifySymlinkOriginPath(symlinkPath *string) string {
	originPath, err := os.Readlink(*symlinkPath)
	if err != nil {
		return ""
	}
	return fmt.Sprintf(" => %s", originPath)
}

func throwRevealDirectoryError() {
	errors.ThrowError(
		"Could not reveal directory.",
		"Ensure that you have enough permissions to read it.",
	)
}
