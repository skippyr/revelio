use std::
{
	env::args,
	path::PathBuf
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

	pub fn get_path(&self) -> Option<PathBuf>
	{
		let last_argument_index: usize = self.arguments.len() - 1;
		if self.has_enough_arguments()
		{ Some(PathBuf::from(self.arguments[last_argument_index].clone())) }
		else
		{ None }
	}
}

