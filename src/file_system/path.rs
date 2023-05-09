use crate::
{
	errors::Error,
	file_system::
	{
		file::File,
		directory::Directory
	}
};
use std::
{
	path::PathBuf,
	fs::Metadata
};

pub struct PathResolver
{ path: PathBuf }

impl PathResolver
{
	pub fn from(path: &PathBuf) -> PathResolver
	{ PathResolver { path: path.clone() } }

	pub fn resolve(&self) -> PathBuf
	{
		match self.path.canonicalize()
		{
			Ok(path) =>
			{ path }
			Err(_error) =>
			{
				Error::new(
					String::from("the given path does not exists."),
					String::from("ensure that you did not mispelled it."),
					1
				).throw();
			}
		}
	}
}

pub struct PathRevealer
{ path: PathBuf }

impl PathRevealer
{
	pub fn from(path: &PathBuf) -> PathRevealer
	{ PathRevealer { path: path.clone() } }

	pub fn reveal(&self)
	{
		let metadata: Metadata = match self.path.metadata()
		{
			Ok(metadata) =>
			{ metadata }
			Err(_error) =>
			{
				Error::new(
					String::from("could not get metadata of given path."),
					String::from("ensure that you have enough permissions to read it."),
					1
				).throw();
			}
		};
		if metadata.is_file()
		{ File::from(&self.path).reveal() }
		else if metadata.is_dir()
		{ Directory::from(&self.path).reveal() }
		else
		{
			Error::new(
				String::from("could not reveal the type of the given path."),
				String::from("read the help instructions by using the flag --help to see what types can be revealed."),
				1
			).throw();
		}
	}
}

