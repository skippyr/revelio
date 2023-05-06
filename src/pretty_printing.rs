pub fn print_error(description: String)
{
	eprintln!("Opsie!");
	eprintln!(
		"\tprogram: reveal.\n\tdescription: {}",
		description
	);
}

pub fn print_help_instructions()
{
	println!("Usage Instructions");
	println!("Starting Point");
	println!("\tAn utility tool to reveal directory entries and file contents.");
	println!("Syntax");
	println!("\tUse it with the following syntax:");
	println!("\t\treveal [flags] <path>");
	println!("\tThe flags it can accept are:");
	println!("\t\t-h or --help - print these help instructions.");
	println!("\tIf no path is provided, it will consider your current one.");
	println!("\tIf multiple paths are provided, it will only consider the last one.");
}

