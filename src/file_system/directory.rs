use std::
{
	fs::
	{
		read_dir,
		ReadDir,
		DirEntry
	},
	path::PathBuf
};
use crate::errors::throw_error;

pub struct Directory
{ stream: ReadDir }

impl Directory
{
	pub fn from(path: &PathBuf) -> Directory
	{ Directory { stream: Directory::get_stream(path) } }

	fn get_stream(path: &PathBuf) -> ReadDir
	{
		match read_dir(path)
		{
			Ok(stream) =>
			{ stream }
			Err(_error) =>
			{
				throw_error(
					String::from("could not read directory."),
					String::from("ensure that you have enough permissions to read it."),
					1
				);
			}
		}
	}

	pub fn reveal(&self)
	{  }
}

