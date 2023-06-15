package main

import (
	"github.com/skippyr/graffiti"
	"os"
	"path/filepath"
	"fmt"
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
	graffiti.Eprintln("Program exited with exit code %d.", exitCode)
	os.Exit(exitCode)
}

func colorizeMode(mode *string) string {
	var coloredMode string
	for characterIterator, character := range *mode {
		if character == '-' {
			coloredMode += string(character)
		} else if characterIterator == 0 {
			coloredMode += fmt.Sprintf("@F{cyan}%c@r", character)
		} else if characterIterator == 1 || characterIterator == 4 || characterIterator == 7 {
			coloredMode += fmt.Sprintf("@F{green}%c@r", character)
		} else if characterIterator == 2 || characterIterator == 5 || characterIterator == 8 {
			coloredMode += fmt.Sprintf("@F{yellow}%c@r", character)
		} else {
			coloredMode += fmt.Sprintf("@F{red}%c@r", character)
		}
	}
	return coloredMode
}

func revealDirectory(path *string) {
	entries, err := os.ReadDir(*path)
	if err != nil {
		throwError(
			"Could not reveal directory.",
			"Ensure that you have enough permissions to read it.",
		)
	}
	graffiti.Println("@UPermissions@r  @UName@r")
	for _, entry := range entries {
		info, err := entry.Info()
		if err != nil {
			continue
		}
		mode := fmt.Sprint(info.Mode())
		graffiti.Println("%s   %s", colorizeMode(&mode), info.Name())
	}
	graffiti.Println("")
	graffiti.Println("@BPath:@r %s.", *path)
	graffiti.Println("@BTotal:@r %d.", len(entries))
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
	if metadata.IsDir() {
		revealDirectory(&path)
	} else {
		throwError(
			"Could not reveal the given path type.",
			"Use the flag --help to see what types can be revealed.",
		)
	}
}

