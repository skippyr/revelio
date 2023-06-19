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
	graffiti.Println("")
	graffiti.Println("@F{yellow}Starting Point")
	graffiti.Println("    The Reveal project is a cross-platform terminal utility to reveal directory")
	graffiti.Println("    entries and file contents.")
	graffiti.Println("")
	graffiti.Println("@F{yellow}Usage")
	graffiti.Println("    Reveal expects a path of a file or directory as argument. For example:")
	graffiti.Println("")
	graffiti.Println("    reveal@r file.txt")
	graffiti.Println("")
	graffiti.Println("    @F{cyan}Revealing A Directory")
	graffiti.Println("        When revealing a directory, Reveal will always give you the following")
	graffiti.Println("        data about its entries:")
	graffiti.Println("            @F{cyan}*@r Size:")
	graffiti.Println("              The size of the entry in the most formidable digital size unit.")
	graffiti.Println("")
	graffiti.Println("            @F{cyan}*@r Kind:")
	graffiti.Println("              The kind of the entry. It can be:")
	graffiti.Println("                  @F{cyan}-@r Directory.")
	graffiti.Println("                  @F{cyan}-@r Device.")
	graffiti.Println("                  @F{cyan}-@r Socket.")
	graffiti.Println("                  @F{cyan}-@r Fifo.")
	graffiti.Println("                  @F{cyan}-@r File.")
	graffiti.Println("                  @F{cyan}-@r Broken.")
	graffiti.Println("              If the given path is a symlink, the kind always points to what")
	graffiti.Println("              it resolves to.")
	graffiti.Println("")
	graffiti.Println("            @F{cyan}*@r Name:")
	graffiti.Println("              The name of the entry.")
	graffiti.Println("              If the given path is a symlink, it will contain the path it")
	graffiti.Println("              points to by its side.")
	graffiti.Println("")
	graffiti.Println("            @F{cyan}*@r Permissions (Unix only):")
	graffiti.Println("              A visual representation of the permissions bits set.")
	graffiti.Println("              For example:")
	graffiti.Println("")
	graffiti.Println("              rwxrwxrwx (777)")
	graffiti.Println("")
	graffiti.Println("              From left to right, it represents the read (r), write (w) and")
	graffiti.Println("              execute (x) permissions for: user, group and others.")
	graffiti.Println("              If a bit is not set, none of these characters will be used.")
	graffiti.Println("")
	graffiti.Println("              It also contains the octal sum of the those bits inside parenthesis.")
	graffiti.Println("")
	graffiti.Println("            @F{cyan}*@r Owner (Unix only):")
	graffiti.Println("              The name of the owner that owns that entry.")
	graffiti.Println("")
	graffiti.Println("            In the end of the output, it will tell how many entries were printed.")
	graffiti.Println("")
	graffiti.Println("    @F{cyan}Revealing A File")
	graffiti.Println("        When revealing a file, Reveal will print all its lines.")
	graffiti.Println("")
	graffiti.Println("        In the end of the output, it will tell how many lines were printed.")
	graffiti.Println("")
	graffiti.Println("@F{yellow}Source Code")
	graffiti.Println("    Reveal's source code can be found at:")
	graffiti.Println("")
	graffiti.Println("    @F{green}https://github.com/skippyr/reveal")
	graffiti.Println("")
	graffiti.Println("@F{yellow}License")
	graffiti.Println("    This project is released under the terms of the MIT license.")
	graffiti.Println("    Copyright (c) 2023, Sherman Rofeman. MIT License.")
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
			"Use the flag --manual to see what types can be revealed.",
		)
	}
}

