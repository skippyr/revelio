package main

import (
	"os"
	"path/filepath"
	"github.com/skippyr/reveal/errors"
	"github.com/skippyr/reveal/file_system"

	"github.com/skippyr/graffiti"
)

func printManual() {
	graffiti.Println("@F{red}Reveal - Manual")
	graffiti.Println("    Starting Point")
	graffiti.Println("        The Reveal project is a cross-platform terminal utility to reveal directory entries and file contents.")
	graffiti.Println("")
	graffiti.Println("    Usage")
	graffiti.Println("        Reveal expects a path of a file or directory as argument.")
	graffiti.Println("        For example:")
	graffiti.Println("")
	graffiti.Println("        reveal file.txt")
	graffiti.Println("")
	graffiti.Println("    Revealing A Directory")
	graffiti.Println("        When revealing a directory, Reveal will always give you the following data about its entries:")
	graffiti.Println("            * Size:")
	graffiti.Println("              The size of the entry in the most formidable digital size unit.")
	graffiti.Println("")
	graffiti.Println("            * Kind:")
	graffiti.Println("              The kind of the entry. It can be:")
	graffiti.Println("                  - Directory.")
	graffiti.Println("                  - Device.")
	graffiti.Println("                  - Socket.")
	graffiti.Println("                  - Fifo.")
	graffiti.Println("                  - File.")
	graffiti.Println("                  - Broken.")
	graffiti.Println("              If the given path is a symlink, the kind always points to what it resolves to.")
	graffiti.Println("")
	graffiti.Println("            * Name:")
	graffiti.Println("              The name of the entry.")
	graffiti.Println("              If the given path is a symlink, it will contain the path it points to by its side.")
	graffiti.Println("")
	graffiti.Println("            * Permissions (Unix only):")
	graffiti.Println("              A visual representation of the permissions bits set.")
	graffiti.Println("              For example:")
	graffiti.Println("")
	graffiti.Println("              rwxrwxrwx (777)")
	graffiti.Println("")
	graffiti.Println("              From left to right, it represents the read (r), write (w) and execute (x) permissions for: user, group and others.")
	graffiti.Println("              If a bit is not set, none of these characters will be used.")
	graffiti.Println("")
	graffiti.Println("              It also contains the octal sum of the those bits inside parenthesis.")
	graffiti.Println("")
	graffiti.Println("            * Owner (Unix only):")
	graffiti.Println("              The name of the owner that owns that entry.")
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
		errors.ThrowError(
			"Could not resolve the absolute path of given path.",
			"Ensure that it uses a valid convention for your system.",
		)
	}
	metadata, err := os.Stat(path)
	if err != nil {
		errors.ThrowError(
			"Could not get metadata of given path.",
			"Ensure that you have not mispelled it.",
		)
	}
	if metadata.IsDir() {
		file_system.RevealDirectory(&path)
	} else if metadata.Mode().IsRegular() {
		file_system.RevealFile(&path)
	} else {
		errors.ThrowError(
			"Could not reveal the given path type.",
			"Use the flag --manual to see what types can be revealed.",
		)
	}
}

