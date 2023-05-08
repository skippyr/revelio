use std::
{
	env::args,
	path::PathBuf,
	process::exit
};
use super::error_treatment::print_error;

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

	pub fn is_to_print_help_instructions(&self) -> bool
	{ self.arguments.contains(&String::from("--help")) }

	pub fn get_path(&self) -> PathBuf
	{
		let last_argument_index: usize = self.arguments.len() - 1;
		let path: PathBuf =
			if self.has_enough_arguments()
			{ PathBuf::from(self.arguments[last_argument_index].clone()) }
			else
			{ PathBuf::from(".") };
		path
			.canonicalize()
			.unwrap_or_else(
				|_error|
				{
					let exit_code: i32 = 1;
					print_error(
						String::from("the given path does not exists."),
						String::from("ensure that you did not mispelled it."),
						exit_code
					);
					exit(exit_code);
				}
			)
	}
}

