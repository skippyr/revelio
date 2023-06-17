package main

import (
	"fmt"
	"io/fs"
	"os"
	"path/filepath"

	"github.com/skippyr/graffiti"
)

type EntryPermission struct {
	bit       int
	character rune
	color     string
}

func throwError(description string, suggestion string) {
	exitCode := 1
	graffiti.Eprintln("@B@F{red}Reveal - Error Report")
	graffiti.Eprintln("    @BDescription")
	graffiti.Eprintln("        %s", description)
	graffiti.Eprintln("")
	graffiti.Eprintln("    @BSuggestion")
	graffiti.Eprintln("        %s", suggestion)
	graffiti.Eprintln("")
	graffiti.Eprintln("    @BHelp@r")
	graffiti.Eprintln("        You can talk to the developer at:")
	graffiti.Eprintln("        @F{green}https://github.com/skippyr/reveal/issues")
	graffiti.Eprintln("")
	graffiti.Eprintln("Program exited with exit code %d.", exitCode)
	os.Exit(exitCode)
}

func stringifyType(typeMode fs.FileMode) string {
	/*
	 * The GoLang uses its own bits to determinate file types.
	 * All bits it has defined can be found here:
	 *
	 * https://pkg.go.dev/io/fs#FileMode
	 *
	 * For UNIX specific, common file types can be found here:
	 *
	 * https://en.wikipedia.org/wiki/Unix_file_types
	 */
	switch {
	case typeMode&fs.ModeDir != 0:
		return "Directory"
	case typeMode&fs.ModeSymlink != 0:
		return "Symlink"
	case typeMode&fs.ModeDevice != 0:
		if typeMode&fs.ModeCharDevice != 0 {
			return "Character"
		} else {
			return "Block"
		}
	case typeMode&fs.ModeNamedPipe != 0:
		return "Fifo"
	case typeMode&fs.ModeSocket != 0:
		return "Socket"
	default:
		return "File"
	}
}

func stringifyPermissions(permissionsMode fs.FileMode) string {
	/*
	 * A list containing all permissions bit for UNIX-like operating systems can be found here:
	 *
	 * https://www.gnu.org/software/libc/manual/html_node/Permission-Bits.html
	 */
	lackPermissionCharacter := '-'
	multipliers := []int{
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
	for _, multiplier := range multipliers {
		for _, permission := range permissions {
			if int(permissionsMode)&(permission.bit*multiplier) != 0 {
				permissionsAsString += fmt.Sprintf("@F{%s}%c@r", permission.color, permission.character)
			} else {
				permissionsAsString += string(lackPermissionCharacter)
			}
		}
	}
	return permissionsAsString
}

func stringifySize(sizeInBytes *int64) string {
	if *sizeInBytes == 0 {
		return "       -"
	}
	const (
		oneGigaByteInBytes = 1e9
		oneMegaByteInBytes = 1e6
		oneKiloByteInBytes = 1e3
		floatPrecision     = 1
		floatDigits        = 6
		intDigits          = 7
		unitColor          = "cyan"
	)
	sizeInGigaBytes := float32(*sizeInBytes) / oneGigaByteInBytes
	if int(sizeInGigaBytes) > 0 {
		return fmt.Sprintf("%*.*fGB", floatDigits, floatPrecision, sizeInGigaBytes)
	}
	sizeInMegaBytes := float32(*sizeInBytes) / oneMegaByteInBytes
	if int(sizeInMegaBytes) > 0 {
		return fmt.Sprintf("%*.*fMB", floatDigits, floatPrecision, sizeInMegaBytes)
	}
	sizeInKiloBytes := float32(*sizeInBytes) / oneKiloByteInBytes
	if int(sizeInKiloBytes) > 0 {
		return fmt.Sprintf("%*.*fkB", floatDigits, floatPrecision, sizeInKiloBytes)
	}
	return fmt.Sprintf("%*dB", intDigits, *sizeInBytes)
}

func revealDirectory(path *string) {
	entries, err := os.ReadDir(*path)
	if err != nil {
		throwError(
			"Could not reveal directory.",
			"Ensure that you have enough permissions to read it.",
		)
	}
	graffiti.Println("    @B@F{red}Size  Permissions       Type  Name")
	for _, entry := range entries {
		info, err := entry.Info()
		if err != nil {
			continue
		}
		var sizeInBytes int64
		if info.Mode().IsRegular() {
			sizeInBytes = info.Size()
		}
		size := stringifySize(&sizeInBytes)
		typeMode := stringifyType(info.Mode().Type())
		permissionsMode := stringifyPermissions(info.Mode().Perm())
		name := graffiti.EscapePrefixCharacters(info.Name())
		graffiti.Println("%s  %s    %9s  %s", size, permissionsMode, typeMode, name)
	}
	graffiti.Println("")
	graffiti.Println("@BPath:@r %s.", graffiti.EscapePrefixCharacters(*path))
	graffiti.Println("@BTotal:@r %d entries.", len(entries))
}

func printManual() {
	graffiti.Println("@B@F{red}Reveal - Manual")
	graffiti.Println("    @BStarting Point")
	graffiti.Println("        Reveal is a program to reveal data about the file system.")
	graffiti.Println("        It can reveal the entries of a directory or the contents of a file.")
	graffiti.Println("        It is created to work on Unix-like operating system: such as Linux and MacOS.")
	graffiti.Println("")
	graffiti.Println("    @BUsage")
	graffiti.Println("        Reveal expects a path for it to reveal.")
	graffiti.Println("        As an example, you can make it reveal all the entries in your home directory:")
	graffiti.Println("")
	graffiti.Println("        @F{red}reveal @F{green}~")
	graffiti.Println("")
	graffiti.Println("        All contents printed contains headers to help you understand the data.")
	graffiti.Println("")
	graffiti.Println("        If it is more comfortable, you can also pipe its output to a pager.")
	graffiti.Println("        For this other example, let's reveal a directory with a large amount of files:")
	graffiti.Println("")
	graffiti.Println("        @F{red}reveal @F{green}/usr/bin@r | @F{red}less")
	graffiti.Println("")
	graffiti.Println("        You can use @F{red}q@r to exit the @F{red}less@r command.")
	graffiti.Println("")
	graffiti.Println("    @BSource Code")
	graffiti.Println("        The source code of this program can be found at:")
	graffiti.Println("        @F{green}https://github.com/skippyr/reveal")
	graffiti.Println("")
	graffiti.Println("    @BLicense")
	graffiti.Println("        Reveal is distributed under the terms of the MIT License.")
	graffiti.Println("        Copyright (c) 2023, Sherman Rofeman. MIT License.")
	os.Exit(0)
}

func main() {
	for argumentsIterator := 0; argumentsIterator < len(os.Args); argumentsIterator++ {
		argument := os.Args[argumentsIterator]
		if argument == "--manual" {
			printManual()
		}
	}
	relativePath := "."
	if len(os.Args) > 1 {
		relativePath = os.Args[1]
	}
	path, err := filepath.Abs(relativePath)
	if err != nil {
		throwError(
			"Could not resolve the absolute path of given path.",
			"Ensure that it uses a valid convention for your system.",
		)
	}
	metadata, err := os.Stat(path)
	if err != nil {
		throwError(
			"Could not get metadata of given path.",
			"Ensure that you have not mispelled it.",
		)
	}
	if metadata.IsDir() {
		revealDirectory(&path)
	} else {
		throwError(
			"Could not reveal the given path type.",
			"Use the flag --manual to see what types can be revealed.",
		)
	}
}
