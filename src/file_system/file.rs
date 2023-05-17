use std::
{
	fs,
	io::
	{
		BufRead,
		Read,
		BufReader
	},
	path::PathBuf
};
use crate::
{
	errors::Error,
	locale::NumberFormatter
};

pub struct File
{
	path: PathBuf,
	descriptor: BufReader<fs::File>
}

impl File
{
	pub fn from(path: &PathBuf) -> File
	{
		File
		{
			path: path.clone(),
			descriptor: File::get_descriptor(path)
		}
	}

	fn get_descriptor(path: &PathBuf) -> BufReader<fs::File>
	{
		let file: fs::File = match fs::File::open(path)
		{
			Ok(file) =>
			{ file }
			Err(_error) => 
			{
				Error::new(
					String::from("could not open the file."),
					String::from("ensure that you have enough permissions to read it.")
				).throw();
			}
		};
		BufReader::new(file)
	}

	pub fn reveal(&mut self)
	{
		let mut line_number: u32 = 1;
		println!(
			"Revealing file: {}.",
			self.path.display()
		);
		for line in self.descriptor.by_ref().lines()
		{
			let line: String = match line
			{
				Ok(line) =>
				{ line }
				Err(_error) =>
				{
					Error::new(
						String::from("could not read lines of the file."),
						String::from("ensure that it is of a readable type.")
					).throw();
				}
			};
			println!(
				"{:>7} | {}",
				NumberFormatter::format_u32(line_number),
				line
			);
			line_number += 1;
		}
	}
}

