use std::{
	process::exit,
	env::args,
	path::PathBuf
};

fn throw_error(description: &str, suggestion: &str) -> ! {
	eprintln!("Error");
	eprintln!("\tDescription:\n\t\t{}", description);
	eprintln!("\tSuggestion:\n\t\t{}", suggestion);
	exit(1);
}

fn main() {
	let arguments = args().collect::<Vec<String>>();
	if arguments.len() < 2 {
		throw_error(
			"Not enough arguments.",
			"Use the --help flag to see what arguments it can receive."
		);
	}
	let path = PathBuf::from(&arguments[1]).canonicalize().unwrap_or_else(|_| {
		throw_error(
			"The given path does not exists.",
			"Ensure that you did not mispell it."
		)
	});
	let metadata = path.metadata().unwrap_or_else(|_| {
		throw_error(
			"Can not get metadata of given path.",
			"Ensure that it is not a broken file."
		);
	});
	if metadata.is_file() {
		eprintln!("Revealing A File");
	} else if metadata.is_dir() {
		eprintln!("Revealing A Directory");
	} else {
		throw_error(
			"Can not reveal this type.",
			"Use the --help flag to see what types can be revealed."
		);
	}
}
