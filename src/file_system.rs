use super::errors::print_error;
use std::
{
	path::PathBuf,
	process::exit
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
				let exit_code: i32 = 1;
				print_error(
					String::from("the given path does not exists."),
					String::from("ensure that you did not mispelled it."),
					exit_code
				);
				exit(exit_code);
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

