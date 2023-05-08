pub fn print_error(
	description: String,
	suggestion: String,
	exit_code: i32
)
{
	eprintln!("Opsie!");
	eprintln!(
		"\tdescription: {}",
		description
	);
	eprintln!(
		"\tsuggestion: {}",
		suggestion
	);
	eprintln!(
		"\texit code: {}",
		exit_code
	);
}

