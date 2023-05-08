use reveal::
{
	arguments_parse::ArgumentsParser,
	file_system::
	{
		File,
		Directory
	},
	error_treatment::print_error
};
use std::
{
	process::exit,
	path::PathBuf,
	fs::Metadata
};

fn print_help_instructions()
{
	println!("Usage Instructions");
	println!("Starting Point");
	println!("\tAn utility tool to reveal directory entries and file contents.");
	println!("Syntax");
	println!("\tUse it with the following syntax:");
	println!("\t\treveal [flags] <path>");
	println!("\tThe flags it can accept are:");
	println!("\t\t--help: print these help instructions.");
	println!("\tIf no path is provided, it will consider your current one.");
	println!("\tIf multiple paths are provided, it will only consider the last one.");
}

fn main()
{
	let arguments_parser: ArgumentsParser = ArgumentsParser::from_environment();
	if arguments_parser.is_to_print_help_instructions()
	{
		print_help_instructions();
		exit(0);
	}
	let path: PathBuf = arguments_parser.get_path();
	let metadata: Metadata = path
		.metadata()
		.unwrap_or_else(
			|_error|
			{
				let exit_code: i32 = 1;
				print_error(
					String::from("could not get path metadata."),
					String::from("ensure that you have enough permissions to read it."),
					exit_code
				);
				exit(exit_code);
			}
		);
	if metadata.is_file()
	{ File::from(&path).reveal(); }
	else if metadata.is_dir()
	{ Directory::from(&path).reveal(); }
	else
	{
		let exit_code: i32 = 1;
		print_error(
			String::from("could not reveal path type."),
			String::from("read the help instructions by using the flag --help."),
			exit_code
		);
		exit(exit_code);
	}
}

