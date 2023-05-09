use users::
{
	User,
	get_user_by_uid
};

pub struct UnixUser
{ name: String }

impl UnixUser
{
	pub fn from(uid: u32) -> Option<UnixUser>
	{
		let user: User = match get_user_by_uid(uid)
		{
			Some(user) =>
			{ user }
			None =>
			{ return None; }
		};
		let name: String = match user.name().to_str()
		{
			Some(name) =>
			{ String::from(name) }
			None =>
			{ return None; }
		};
		Some(UnixUser { name })
	}

	pub fn get_name(&self) -> String
	{ self.name.clone() }
}

