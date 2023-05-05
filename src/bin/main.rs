use reveal::arguments_parser::ArgumentsParser;
use std::process::exit;

fn print_usage_instructions()
{
	eprintln!("Usage Instructions");
	eprintln!("Starting Point");
	eprintln!("Syntax");
}

fn main()
{
	let arguments_parser: ArgumentsParser = ArgumentsParser::from_environment();
	if !arguments_parser.has_enough_arguments()
	{
		print_usage_instructions();
		exit(1);
	}
	eprintln!("{:?}", arguments_parser.get_path());
}
