use std::{
	process::exit,
	env::args,
	path::PathBuf,
	fs::File,
	io::{
		BufReader,
		BufRead
	}
};

fn throw_error(description: &str, suggestion: &str) -> ! {
	eprintln!("Error");
	eprintln!("\tDescription\n\t\t{}", description);
	eprintln!("\tSuggestion\n\t\t{}", suggestion);
	exit(1);
}

fn print_help_instructions() -> ! {
	println!("Reveal - Help Instructions");
	println!("\tStarting Point");
	println!("\t\tA program to reveal directory entries and file contents.");
	println!("\tSyntax");
	println!("\t\treveal [flags] <path>");
	println!("\tFlags");
	println!("\t\t--help: print these help instructions.");
	exit(0);
}

fn reveal_file(path: &PathBuf) {
	let file = File::open(path).unwrap_or_else(|_| {
		throw_error(
			"Could not open file.",
			"Ensure that you have enough permissions to read it."
		);
	});
	let reader = BufReader::new(file);
	for line in reader.lines() {
		let line = line.unwrap_or_else(|_| {
			throw_error(
				"Could not read lines of file.",
				"Ensure that the file is not a binary."
			);
		});
		eprintln!("{}", line);
	}
}

fn main() {
	let arguments = args().collect::<Vec<String>>();
	if arguments.contains(&String::from("--help")) {
		print_help_instructions();
	}
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
		);
	});
	let metadata = path.metadata().unwrap_or_else(|_| {
		throw_error(
			"Can not get metadata of given path.",
			"Ensure that it is not a broken file."
		);
	});
	if metadata.is_file() {
		reveal_file(&path);
	} else if metadata.is_dir() {
		eprintln!("Revealing A Directory");
	} else {
		throw_error(
			"Can not reveal this type.",
			"Use the --help flag to see what types can be revealed."
		);
	}
}
