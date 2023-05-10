use std::process::exit;

pub struct Helper;

impl Helper
{
	pub fn print_instructions() -> !
	{
		eprintln!("Help Instructions - Reveal");
		eprintln!("\tStarting Point");
		eprintln!("\t\tA program to reveal directory entries and file contents for UNIX-like operating systems.");
		eprintln!("\tSyntax");
		eprintln!("\t\tUse this program with following syntax:");
		eprintln!("\t\t\treveal [flags] <path>");
		eprintln!("\t\tThe flags it can accept are:");
		eprintln!("\t\t\t--help: print these help instructions.");
		eprintln!("\t\tIf no path is provided, it will consider your current directory.");
		eprintln!("\t\tIf multiple paths are provided, only the last one will be considered.");
		exit(0);
	}
}

