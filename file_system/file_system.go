package file_system

import (
	"io/fs"
	"os"
	"fmt"
	"bufio"
	"unicode/utf8"
	"strings"
	"github.com/skippyr/graffiti"

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
	if mode == 0 {
		return "Broken"
	}
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

func RevealFile(filePath *string) {
	file, err := os.Open(*filePath)
	if err != nil {
		errors.ThrowError(
			"Could not open file.",
			"Ensure that you have enough permissions to read it.",
		)
	}
	defer file.Close()
	scanner := bufio.NewScanner(file)
	scanner.Split(bufio.ScanRunes)
	var quantityOfLines uint
	for scanner.Scan() {
		if !utf8.ValidString(scanner.Text()) {
			errors.ThrowError(
				"Could not read file as it contains non UTF-8 encoded characters.",
				"Ensure that it is of a readable type.",
			)
		}
		if scanner.Text() == "\n" {
			quantityOfLines ++
		}
		graffiti.Print(strings.ReplaceAll(graffiti.EscapePrefixCharacters(scanner.Text()), "\x1b", "@K{white}@F{black}[ESCAPE]@r"))
	}
	graffiti.Println("")
	graffiti.Println("Path: %s.", graffiti.EscapePrefixCharacters(*filePath))
	graffiti.Println("Total: %d lines.", quantityOfLines)
}
