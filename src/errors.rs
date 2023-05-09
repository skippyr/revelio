use std::process::exit;

pub struct Error
{
	description: String,
	suggestion: String,
	exit_code: u32
}

impl Error
{
	pub fn new(
		description: String,
		suggestion: String,
		exit_code: u32
	) -> Error
	{
		Error
		{
			description,
			suggestion,
			exit_code
		}
	}

	pub fn throw(&self) -> !
	{
		eprintln!("Opsie!");
		eprintln!(
			"\tdescription: {}",
			self.description
		);
		eprintln!(
			"\tsuggestion: {}",
			self.suggestion
		);
		eprintln!(
			"\texit code: {}.",
			self.exit_code
		);
		exit(self.exit_code as i32);
	}
}

