use std::
{
	fs,
	path::PathBuf,
	process::exit, io::{BufReader, BufRead}
};
use super::pretty_printing::print_error;

pub struct File
{ path: PathBuf }

impl File
{
	pub fn from(path: &PathBuf) -> File
	{ File { path: path.clone() } }

	pub fn reveal(&self)
	{
		let file: fs::File = fs::File::open(&self.path).unwrap_or_else(
			|_error|
			{
				print_error(String::from("Could not open file."));
				exit(1);
			}
		);
		let buffer: BufReader<fs::File> = BufReader::new(file);
		let mut line_number: u32 = 1;
		for line in buffer.lines()
		{
			let line: String = line.unwrap_or_else(
				|_error|
				{
					print_error(String::from("Could not read lines of file."));
					exit(1);
				}
			);
			println!(
				"{:>5} | {}",
				line_number,
				line
			);
			line_number += 1;
		}
	}
}


