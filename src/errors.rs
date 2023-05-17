use std::process::exit;

pub struct Error
{
	description: String,
	suggestion: String
}

impl Error
{
	pub fn new(
		description: String,
		suggestion: String
	) -> Error
	{
		Error
		{
			description,
			suggestion
		}
	}

	pub fn throw(&self) -> !
	{
		eprintln!("Opsie!");
		eprintln!("\tprogram: reveal.");
		eprintln!(
			"\tdescription: {}",
			self.description
		);
		eprintln!(
			"\tsuggestion: {}",
			self.suggestion
		);
		exit(1);
	}
}

