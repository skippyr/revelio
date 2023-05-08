use crate::
{
	errors::throw_error,
	file_system::file::File
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
				throw_error(
					String::from("the given path does not exists."),
					String::from("ensure that you did not mispelled it."),
					1
				);
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
				throw_error(
					String::from("could not get metadata of given path."),
					String::from("ensure that you have enough permissions to read it."),
					1
				);
			}
		};
		if metadata.is_file()
		{ File::from(&self.path).reveal() }
		else
		{
			throw_error(
				String::from("could not reveal the type of the given path."),
				String::from("read the help instructions by using the flag --help."),
				1
			);
		}
	}
}

