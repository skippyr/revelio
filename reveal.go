package main

import (
	"os"
	"path/filepath"
	"github.com/skippyr/reveal/errors"
	"github.com/skippyr/reveal/file_system"

	"github.com/skippyr/graffiti"
)

func printManual() {
	graffiti.Println("Reveal - Manual")
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
	} else {
		errors.ThrowError(
			"Could not reveal the given path type.",
			"Use the flag --manual to see what types can be revealed.",
		)
	}
}

