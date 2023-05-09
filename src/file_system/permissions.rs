pub struct UnixPermissions
{ permissions_mode: u32 }

const UNIX_OWNER_READING_PERMISSIONS_BIT: u32 = 0o400;
const UNIX_OWNER_WRITING_PERMISSIONS_BIT: u32 = 0o200;
const UNIX_OWNER_EXECUTION_PERMISSIONS_BIT: u32 = 0o100;
const UNIX_GROUP_READING_PERMISSIONS_BIT: u32 = 0o40;
const UNIX_GROUP_WRITING_PERMISSIONS_BIT: u32 = 0o20;
const UNIX_GROUP_EXECUTION_PERMISSIONS_BIT: u32 = 0o10;
const UNIX_OTHERS_READING_PERMISSIONS_BIT: u32 = 0o4;
const UNIX_OTHERS_WRITING_PERMISSIONS_BIT: u32 = 0o2;
const UNIX_OTHERS_EXECUTION_PERMISSIONS_BIT: u32 = 0o1;

impl UnixPermissions
{
	pub fn from(permissions_mode: u32) -> UnixPermissions
	{ UnixPermissions { permissions_mode } }

	fn does_owner_can_read(&self) -> bool
	{ self.permissions_mode & UNIX_OWNER_READING_PERMISSIONS_BIT != 0 }

	fn does_owner_can_write(&self) -> bool
	{ self.permissions_mode & UNIX_OWNER_WRITING_PERMISSIONS_BIT != 0 }

	fn does_owner_can_execute(&self) -> bool
	{ self.permissions_mode & UNIX_OWNER_EXECUTION_PERMISSIONS_BIT != 0 }

	fn does_group_can_read(&self) -> bool
	{ self.permissions_mode & UNIX_GROUP_READING_PERMISSIONS_BIT != 0 }

	fn does_group_can_write(&self) -> bool
	{ self.permissions_mode & UNIX_GROUP_WRITING_PERMISSIONS_BIT != 0 }

	fn does_group_can_execute(&self) -> bool
	{ self.permissions_mode & UNIX_GROUP_EXECUTION_PERMISSIONS_BIT != 0 }

	fn does_others_can_read(&self) -> bool
	{ self.permissions_mode & UNIX_OTHERS_READING_PERMISSIONS_BIT != 0 }

	fn does_others_can_write(&self) -> bool
	{ self.permissions_mode & UNIX_OTHERS_WRITING_PERMISSIONS_BIT != 0 }

	fn does_others_can_execute(&self) -> bool
	{ self.permissions_mode & UNIX_OTHERS_EXECUTION_PERMISSIONS_BIT != 0 }

	pub fn as_bits_sum(&self) -> u32
	{
		let mut sum: u32 = 0;
		if self.does_owner_can_read()
		{ sum += UNIX_OWNER_READING_PERMISSIONS_BIT; }
		if self.does_owner_can_write()
		{ sum += UNIX_OWNER_WRITING_PERMISSIONS_BIT; }
		if self.does_owner_can_execute()
		{ sum += UNIX_OWNER_EXECUTION_PERMISSIONS_BIT; }
		if self.does_group_can_read()
		{ sum += UNIX_GROUP_READING_PERMISSIONS_BIT; }
		if self.does_group_can_write()
		{ sum += UNIX_GROUP_WRITING_PERMISSIONS_BIT; }
		if self.does_group_can_execute()
		{ sum += UNIX_GROUP_EXECUTION_PERMISSIONS_BIT; }
		if self.does_others_can_read()
		{ sum += UNIX_OTHERS_READING_PERMISSIONS_BIT; }
		if self.does_others_can_write()
		{ sum += UNIX_OTHERS_WRITING_PERMISSIONS_BIT; }
		if self.does_others_can_execute()
		{ sum += UNIX_OTHERS_EXECUTION_PERMISSIONS_BIT; }
		sum
	}

	pub fn as_string(&self) -> String
	{
		const READING_CHARACTER: char = 'r';
		const WRITING_CHARACTER: char = 'w';
		const EXECUTION_CHARACTER: char = 'x';
		const NONE_CHARACTER: char = '-';
		let mut string: String = String::new();
		if self.does_owner_can_read()
		{ string.push(READING_CHARACTER); }
		else { string.push(NONE_CHARACTER); }
		if self.does_owner_can_write()
		{ string.push(WRITING_CHARACTER); }
		else { string.push(NONE_CHARACTER); }
		if self.does_owner_can_execute()
		{ string.push(EXECUTION_CHARACTER); }
		else { string.push(NONE_CHARACTER); }
		if self.does_group_can_read()
		{ string.push(READING_CHARACTER); }
		else { string.push(NONE_CHARACTER); }
		if self.does_group_can_write()
		{ string.push(WRITING_CHARACTER); }
		else { string.push(NONE_CHARACTER); }
		if self.does_group_can_execute()
		{ string.push(EXECUTION_CHARACTER); }
		else { string.push(NONE_CHARACTER); }
		if self.does_others_can_read()
		{ string.push(READING_CHARACTER); }
		else { string.push(NONE_CHARACTER); }
		if self.does_others_can_write()
		{ string.push(WRITING_CHARACTER); }
		else { string.push(NONE_CHARACTER); }
		if self.does_others_can_execute()
		{ string.push(EXECUTION_CHARACTER); }
		else { string.push(NONE_CHARACTER); }
		string
	}
}

