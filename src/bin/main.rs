use reveal::arguments_parser::ArgumentsParser;
use std::
{
	process::exit,
	path::PathBuf
};

fn print_error(message: &str)
{
	eprintln!("Opsie!");
	eprintln!(
		"{}",
		message
	);
}

fn print_usage_instructions()
{
	eprintln!("Usage Instructions");
	eprintln!("Starting Point");
	eprintln!("Syntax");
}

fn main()
{
	let arguments_parser: ArgumentsParser = ArgumentsParser::from_environment();
	let path: PathBuf = arguments_parser
		.get_path()
		.unwrap_or_else(
			||
			{
				print_usage_instructions();
				exit(1);
			}
		)
		.canonicalize()
		.unwrap_or_else(
			|_error|
			{
				print_error("The path does not exist.");
				exit(1);
			}
		);
	eprintln!("{:?}", path);
}
