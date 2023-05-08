use crate::errors::throw_error;
use std::path::PathBuf;

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
	{  }
}

