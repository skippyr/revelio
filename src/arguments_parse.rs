use std::
{
	env::args,
	path::PathBuf,
	process::exit
};
use super::pretty_printing::
{
	print_error,
	print_usage_instructions
};

pub struct ArgumentsParser
{ arguments: Vec<String> }

impl ArgumentsParser
{
	pub fn from_environment() -> ArgumentsParser
	{ ArgumentsParser { arguments: args().collect() } }

	fn has_enough_arguments(&self) -> bool
	{
		const DEFAULT_ARGUMENTS_LENGTH: usize = 1;
		self.arguments.len() > DEFAULT_ARGUMENTS_LENGTH
	}

	pub fn get_path(&self) -> PathBuf
	{
		if !self.has_enough_arguments()
		{
			print_usage_instructions();
			exit(1);
		}
		let last_argument_index: usize = self.arguments.len() - 1;
		PathBuf::from(self.arguments[last_argument_index].clone())
			.canonicalize()
			.unwrap_or_else(
				|_error|
				{
					print_error(String::from("The path does not exists."));
					exit(1);
				}
			)
	}
}

