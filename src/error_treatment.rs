pub fn print_error(
	description: String,
	suggestion: String,
	exit_code: i32
)
{
	eprintln!("Opsie!");
	eprintln!("\tprogram: reveal.");
	eprintln!(
		"\tdescription: {}",
		description
	);
	eprintln!(
		"\tsuggestion: {}",
		suggestion
	);
	eprintln!(
		"\texit code: {}.",
		exit_code
	);
}

