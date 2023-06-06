use std::process::exit;

pub fn throw_error(description: &str, suggestion: &str) -> ! {
	eprintln!("Reveal - Error Report");
	eprintln!("\tDescription:\n\t\t{}", description);
	eprintln!("\tSuggestion(s):\n\t\t* {}", suggestion);
	eprintln!("Program exited with code 1.");
	exit(1);
}

