package errors

import (
	"os"

	"github.com/skippyr/graffiti"
)

func ThrowError(description string, suggestion string) {
	exitCode := 1
	graffiti.Eprintln("@F{red}Reveal - Error Report")
	graffiti.Eprintln("")
	graffiti.Eprintln("@F{yellow}Description")
	graffiti.Eprintln("    %s", description)
	graffiti.Eprintln("")
	graffiti.Eprintln("@F{yellow}Suggestion")
	graffiti.Eprintln("    %s", suggestion)
	graffiti.Eprintln("")
	graffiti.Eprintln("@F{yellow}Help")
	graffiti.Eprintln("    If you need more information about usage,")
	graffiti.Eprintln("    you can read its manual by using the manual flag:")
	graffiti.Eprintln("")
	graffiti.Eprintln("    reveal --manual")
	graffiti.Eprintln("")
	graffiti.Eprintln("    If you can not figure out how to solve this error,")
	graffiti.Eprintln("    You can file an issue describing what happened at:")
	graffiti.Eprintln("")
	graffiti.Eprintln("    @F{green}https://github.com/skippyr/reveal/issues")
	graffiti.Eprintln("")
	graffiti.Eprintln("    And the developer will help you whenever possible.")
	graffiti.Eprintln("")
	graffiti.Eprintln("Program exited with exit code %d.", exitCode)
	os.Exit(exitCode)
}
