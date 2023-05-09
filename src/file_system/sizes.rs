pub struct DigitalSize
{ bytes: u64 }

impl DigitalSize
{
	pub fn from(bytes: u64) -> DigitalSize
	{ DigitalSize { bytes } }

	fn to_gigabytes(&self) -> f32
	{
		const ONE_GIGABYTE_AS_BYTES: u64 = 10_u64.pow(9);
		self.bytes as f32 / ONE_GIGABYTE_AS_BYTES as f32
	}

	fn to_megabytes(&self) -> f32
	{
		const ONE_MEGABYTE_AS_BYTES: u64 = 10_u64.pow(6);
		self.bytes as f32 / ONE_MEGABYTE_AS_BYTES as f32
	}

	fn to_kilobytes(&self) -> f32
	{
		const ONE_KILOBYTE_AS_BYTES: u64 = 10_u64.pow(3);
		self.bytes as f32 / ONE_KILOBYTE_AS_BYTES as f32
	}

	pub fn as_string(&self) -> String
	{
		if self.to_gigabytes() as u64 > 0
		{
			format!(
				"{:.1}GB",
				self.to_gigabytes()
			)
		}
		else if self.to_megabytes() as u64 > 0
		{
			format!(
				"{:.1}MB",
				self.to_megabytes()
			)
		}
		else if self.to_kilobytes() as u64 > 0
		{
			format!(
				"{:.1}kB",
				self.to_kilobytes()
			)
		}
		else if self.bytes > 0
		{
			format!(
				"{}B",
				self.bytes
			)
		}
		else
		{ String::from("-") }
	}
}

