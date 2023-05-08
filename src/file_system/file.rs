use std::
{
	fs,
	io::
	{
		BufRead,
		BufReader
	},
	path::PathBuf
};

use crate::errors::throw_error;

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
				throw_error(
					String::from("could not open the file."),
					String::from("ensure that you have enough 	permissions to read it."),
					1
				);
			}
		};
		BufReader::new(file)
	}

	pub fn reveal(&self)
	{  }
}

