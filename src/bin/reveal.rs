use reveal::
{
	arguments::ArgumentsParser,
	help::Helper,
	file_system::path::
	{
		PathResolver,
		PathRevealer
	}
};
use std::path::PathBuf;

fn main()
{
	let arguments_parser: ArgumentsParser = ArgumentsParser::from_environment();
	if arguments_parser.is_to_print_help_instructions()
	{ Helper::print_instructions(); }
	let unresolved_path: PathBuf = arguments_parser.get_path();
	let path_resolver: PathResolver = PathResolver::from(&unresolved_path);
	let path: PathBuf = path_resolver.resolve();
	let path_revealer: PathRevealer = PathRevealer::from(&path);
	path_revealer.reveal();
}

