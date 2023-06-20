package main

import (
	"os"
	"path/filepath"
	"github.com/skippyr/reveal/errors"
	"github.com/skippyr/reveal/file_system"

	"github.com/skippyr/graffiti"
)

func printManual() {
	defer os.Exit(0)
	graffiti.Println("@F{magenta}Reveal - Manual")
	graffiti.Println("")
	graffiti.Println("@F{magenta}Starting Point")
	graffiti.Println("    The Reveal project is a cross-platform terminal utility to reveal directory")
	graffiti.Println("    entries and file contents.")
	graffiti.Println("")
	graffiti.Println("@F{magenta}Usage")
	graffiti.Println("    Reveal expects the path of a file or directory as argument. For example:")
	graffiti.Println("")
	graffiti.Println("    reveal file.txt")
	graffiti.Println("")
	graffiti.Println("    If the name contains spaces, wrap it around quotes: single or double. For")
	graffiti.Println("    example:")
	graffiti.Println("")
	graffiti.Println("    reveal \"file with spaces.txt\"")
	graffiti.Println("")
	graffiti.Println("    @F{magenta}Revealing A Directory")
	graffiti.Println("        When revealing a directory, Reveal will give you the following data")
	graffiti.Println("        about its entries:")
	graffiti.Println("            * Size:")
	graffiti.Println("              The size of the entry using the byte unit. It will use the most")
	graffiti.Println("              formidable multiplier prefix.")
	graffiti.Println("")
	graffiti.Println("            * Kind:")
	graffiti.Println("              The kind of the entry. It can be:")
	graffiti.Println("                  - Directory.")
	graffiti.Println("                  - Device.")
	graffiti.Println("                  - Socket.")
	graffiti.Println("                  - Fifo.")
	graffiti.Println("                  - File.")
	graffiti.Println("                  - Broken.")
	graffiti.Println("")
	graffiti.Println("              If the given path is a symlink, the kind always points to what")
	graffiti.Println("              it resolves to.")
	graffiti.Println("")
	graffiti.Println("            * Name:")
	graffiti.Println("              The name of the entry.")
	graffiti.Println("")
	graffiti.Println("              If the given path is a symlink, it will contain the path it")
	graffiti.Println("              points to by its side.")
	graffiti.Println("")
	graffiti.Println("            * Permissions (Unix only):")
	graffiti.Println("              A visual representation of the permissions bits set. For example:")
	graffiti.Println("")
	graffiti.Println("              rwxrwxrwx (777)")
	graffiti.Println("")
	graffiti.Println("              From left to right, it represents the read (r), write (w) and")
	graffiti.Println("              execute (x) permissions for: user, group and others.")
	graffiti.Println("")
	graffiti.Println("              If a bit is not set, none of these characters will be used.")
	graffiti.Println("")
	graffiti.Println("              It also contains the octal sum of those bits inside parenthesis.")
	graffiti.Println("")
	graffiti.Println("            * Owner (Unix only):")
	graffiti.Println("              The name of the owner that owns that entry.")
	graffiti.Println("")
	graffiti.Println("            In the end of the output, it will tell how many entries were printed.")
	graffiti.Println("")
	graffiti.Println("    @F{magenta}Revealing A File")
	graffiti.Println("        When revealing a file, Reveal will print its contents.")
	graffiti.Println("")
	graffiti.Println("        In the end of the output, it will tell how many lines were printed.")
	graffiti.Println("")
	graffiti.Println("        Revealing large files can take some time. If you want to interrupt a reading,")
	graffiti.Println("        send a SIGINT signal to your shell by using the keys sequence: [Ctrl] + [c].")
	graffiti.Println("")
	graffiti.Println("@F{magenta}Version")
	graffiti.Println("    Check what version you are using by using the --version flag:")
	graffiti.Println("")
	graffiti.Println("    reveal --version")
	graffiti.Println("")
	graffiti.Println("@F{magenta}Source Code")
	graffiti.Println("    Reveal's source code can be found at:")
	graffiti.Println("")
	graffiti.Println("    @Uhttps://github.com/skippyr/reveal")
	graffiti.Println("")
	graffiti.Println("@F{magenta}License")
	graffiti.Println("    This project is released under the terms of the MIT license.")
	graffiti.Println("")
	graffiti.Println("    Copyright (c) 2023, Sherman Rofeman. MIT License.")
}

func printVersion() {
	defer os.Exit(0)
	graffiti.Println("v2.0.0")
}

func main() {
	for argumentsIterator := 0; argumentsIterator < len(os.Args); argumentsIterator++ {
		argument := os.Args[argumentsIterator]
		if argument == "--version" {
			printVersion()
		} else if argument == "--manual" {
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
			"Ensure that you current directory is valid.",
		)
	}
	metadata, err := os.Stat(path)
	if err != nil {
		errors.ThrowError(
			"The given path does not exists.",
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
			"Check out Reveal's manual to see what types can be revealed.",
		)
	}
}

