use users::get_user_by_uid;

pub struct UnixUser
{ name: String }

impl UnixUser
{
	pub fn from(uid: u32) -> UnixUser
	{
		let mut name: String = String::from("-");
		if let Some(user) = get_user_by_uid(uid)
		{
			if let Some(name_as_osstr) = user.name().to_str()
			{ name = String::from(name_as_osstr) }
		};
		UnixUser { name }
	}

	pub fn get_name(&self) -> String
	{ self.name.clone() }
}

