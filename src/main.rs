use std::{
	env::args,
	process::exit
};

fn print_error(message: &str)
{
	eprintln!("Opsie!");
	eprintln!(
		"\treveal: {}",
		message
	);
	eprintln!("\tUse the -h or --help flags to print help instructions.");
	return;
}

fn print_help_instructions()
{
	eprintln!("Help Instructions");
	eprintln!("\tStarting Point");
	eprintln!("\t\tReveal is an utility tool to reveal directory entries and file contents.");
	eprintln!("\tSyntax");
	eprintln!("\t\tUse it with the following syntax:");
	eprintln!("\t\t\treveal <path>");
	eprintln!("\t\tIf no path is given, the current directory one will be considered.");
	eprintln!("\t\tIf multiple paths are given, only the last one will be revealed.");
	return;
}

fn main()
{
	let arguments: Vec<String> = args().collect();
	let mut relative_path: String = String::from(".");
	if arguments.len() == 1
	{
		print_error("Not enough arguments.");
		exit(1);
	}
	for arguments_iterator in 1..arguments.len()
	{
		if
			arguments[arguments_iterator] == "-h" ||
			arguments[arguments_iterator] == "--help"
		{
			print_help_instructions();
			exit(0);
		}
		else
		{
			relative_path = arguments[arguments_iterator].clone();
		}
	}
	return;
}

