use std::process::exit;

pub fn throw_error(description: &str, suggestion: &str) -> ! {
	eprintln!("Error");
	eprintln!("\tDescription\n\t\t{}", description);
	eprintln!("\tSuggestion\n\t\t{}", suggestion);
	exit(1);
}

