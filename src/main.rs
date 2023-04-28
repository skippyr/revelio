use std::{
	env::args,
	process::exit,
	fs::{
		canonicalize,
		Metadata
	},
	path::PathBuf
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

fn reveal_directory(directory_path: &PathBuf)
{
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
	let absolute_path: PathBuf = match canonicalize(&relative_path)
	{
		Ok(absolute_path) =>
		{
			absolute_path
		}
		Err(_) =>
		{
			print_error(&format!(
				"Could not find given path: \"{}\".",
				relative_path
			));
			exit(1);
		}
	};
	let absolute_path_metadata: Metadata = match absolute_path.metadata()
	{
		Ok(absolute_path_metadata) =>
		{
			absolute_path_metadata
		}
		Err(_) =>
		{
			print_error(&format!(
				"Could not get metadata of given path: \"{}\".",
				relative_path
			));
			exit(1);
		}
	};
	if absolute_path_metadata.is_dir()
	{
		reveal_directory(&absolute_path);
	}
	else
	{
		print_error(&format!(
			"Could not reveal file type of given path: \"{}\".",
			relative_path
		));
		exit(1);
	}
	return;
}

