use std::{
	process::exit,
	env::args,
	path::PathBuf
};
use reveal::{
	error_handling::throw_error,
	file_system::{
		reveal_file,
		reveal_directory
	}
};

fn print_help_instructions() -> ! {
	println!("Reveal - Help Instructions");
	println!("\tStarting Point:");
	println!("\t\tReveal is a program to reveal directory entries and file contents.");
	println!("\tSyntax:");
	println!("\t\treveal [flags] <path>");
	println!("\tFlags:");
	println!("\t\t--help: print these help instructions.");
	exit(0);
}

fn main() {
	let arguments = args().collect::<Vec<String>>();
	if arguments.contains(&String::from("--help")) {
		print_help_instructions();
	}
	let path = PathBuf::from(if arguments.len() < 2 {
		"."
	} else {
		&arguments[1]
	}).canonicalize().unwrap_or_else(|_| {
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
		reveal_directory(&path);
	} else {
		throw_error(
			"Can not reveal this type.",
			"Use the --help flag to see what types can be revealed."
		);
	}
}
