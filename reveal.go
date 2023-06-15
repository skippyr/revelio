package main

import (
	"github.com/skippyr/graffiti"
	"os"
	"path/filepath"
)

func throwError(description string, suggestion string) {
	exitCode := 1
	graffiti.Eprintln("@B@F{red}Reveal - Error Report")
	graffiti.Eprintln("    @BDescription")
	graffiti.Eprintln("        %s", description)
	graffiti.Eprintln("")
	graffiti.Eprintln("    @BSuggestion")
	graffiti.Eprintln("        %s", suggestion)
	graffiti.Eprintln("")
	graffiti.Eprintln("Program exited with exit code @F{cyan}%d@r.", exitCode)
	os.Exit(exitCode)
}

func main() {
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
	graffiti.Println("Is directory: %v.", metadata.IsDir())
	graffiti.Println("Path: %s.", path)
}

