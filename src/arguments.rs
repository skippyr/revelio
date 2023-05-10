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

	
	pub fn is_to_print_help_instructions(&self) -> bool
	{ self.arguments.contains(&String::from("--help")) }
	
	fn is_flag(
		&self,
		argument: &str
	) -> bool
	{
		let characters: Vec<char> = argument.chars().collect();
		characters.len() > 2 &&
		characters[0] == '-' &&
		characters[1] == '-'
	}

	pub fn get_path(&self) -> PathBuf
	{
		let mut path: String = String::from(".");
		for arguments_iterator in 1..self.arguments.len()
		{
			let argument: String =  self.arguments[arguments_iterator].clone();
			if !self.is_flag(&argument)
			{ path = argument }
		}
		PathBuf::from(path)
	}
}

