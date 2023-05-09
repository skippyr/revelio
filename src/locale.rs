use num_format::
{
	Locale,
	ToFormattedString
};

pub struct NumberFormatter;

const LOCALE: Locale = Locale::en;

impl NumberFormatter
{
	pub fn format_u32(number: u32) -> String
	{ number.to_formatted_string(&LOCALE) }
}

