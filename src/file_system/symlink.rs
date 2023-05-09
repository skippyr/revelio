use std::
{
	fs::read_link,
	path::PathBuf
};

pub struct Symlink
{ path: Option<PathBuf> }

impl Symlink
{
	pub fn from(path: &PathBuf) -> Symlink
	{
		Symlink
		{
			path: match read_link(path)
			{
				Ok(path) =>
				{ Some(path) }
				Err(_error) =>
				{ None }
			}
		}
	}

	pub fn as_decorator_string(&self) -> String
	{
		match &self.path
		{
			Some(_symlink_path) =>
			{ String::from("@") }
			None =>
			{ String::from(" ") }
		}
	}

	pub fn as_string(&self) -> String
	{
		match &self.path
		{
			Some(symlink_path) =>
			{
				format!(
					" -> {}",
					symlink_path.display()
				)
			}
			None =>
			{ String::new() }
		}
	}
}

