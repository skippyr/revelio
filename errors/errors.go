package errors

import (
	"os"

	"github.com/skippyr/graffiti"
)

func ThrowError(description string, suggestion string) {
	exitCode := 1
	defer os.Exit(exitCode)
	graffiti.Eprintln("@F{magenta}Reveal - Error Report")
	graffiti.Eprintln("")
	graffiti.Eprintln("@F{magenta}Description")
	graffiti.Eprintln("    %s", description)
	graffiti.Eprintln("")
	graffiti.Eprintln("@F{magenta}Suggestion")
	graffiti.Eprintln("    %s", suggestion)
	graffiti.Eprintln("")
	graffiti.Eprintln("@F{magenta}Help")
	graffiti.Eprintln("    If you need more information about usage, you can read Reveal's manual by")
	graffiti.Eprintln("    using the manual flag:")
	graffiti.Eprintln("")
	graffiti.Eprintln("    reveal --manual")
	graffiti.Eprintln("")
	graffiti.Eprintln("    If you can not figure out how to solve this error, you can file an issue")
	graffiti.Eprintln("    describing what happened at:")
	graffiti.Eprintln("")
	graffiti.Eprintln("    @Uhttps://github.com/skippyr/reveal/issues")
	graffiti.Eprintln("")
	graffiti.Eprintln("    And the Reveal's developer will help you whenever possible.")
	graffiti.Eprintln("")
	graffiti.Eprintln("Program exited with exit code %d.", exitCode)
}
