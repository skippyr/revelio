use reveal::{
	arguments_parse::ArgumentsParser,
	pretty_printing::print_error,
	file_system::
	{
		File,
		Directory
	}
};
use std::
{
	process::exit,
	path::PathBuf,
	fs::Metadata
};

fn main()
{
	let arguments_parser: ArgumentsParser = ArgumentsParser::from_environment();
	let path: PathBuf = arguments_parser.get_path();
	let metadata: Metadata = path
		.metadata()
		.unwrap_or_else(
			|_error|
			{
				print_error(String::from("Could not get path metadata."));
				exit(1);
			}
		);
	if metadata.is_file()
	{ File::from(&path).reveal(); }
	else if metadata.is_dir()
	{ Directory::from(&path).reveal(); }
	else
	{
		print_error(String::from("Could not reveal path type."));
		exit(1);
	}
}
