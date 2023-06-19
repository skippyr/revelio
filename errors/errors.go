package errors

import (
	"os"

	"github.com/skippyr/graffiti"
)

func ThrowError(description string, suggestion string) {
	exitCode := 1
	graffiti.Eprintln("@F{red}Reveal - Error Report")
	graffiti.Eprintln("    Description")
	graffiti.Eprintln("        %s", description)
	graffiti.Eprintln("")
	graffiti.Eprintln("    Suggestion")
	graffiti.Eprintln("        %s", suggestion)
	graffiti.Eprintln("")
	graffiti.Eprintln("    Help")
	graffiti.Eprintln("        You can talk to the developer at:")
	graffiti.Eprintln("        https://github.com/skippyr/reveal/issues")
	graffiti.Eprintln("")
	graffiti.Eprintln("Program exited with exit code %d.", exitCode)
	os.Exit(exitCode)
}
