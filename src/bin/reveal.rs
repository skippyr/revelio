use reveal::
{
	arguments::ArgumentsParser,
	help::print_help_instructions
};
use std::
{
	process::exit,
	path::PathBuf
};

fn main()
{
	let arguments_parser: ArgumentsParser = ArgumentsParser::from_environment();
	if arguments_parser.is_to_print_help_instructions()
	{
		print_help_instructions();
		exit(0);
	}
	let path: PathBuf = arguments_parser.get_path();
	eprintln!(
		"{:?}",
		path
	);
}

