use std::{
	fs::{
		File,
		read_dir,
		read_link,
		FileType
	},
	path::PathBuf,
	io::{
		BufRead,
		BufReader
	},
	fmt::{
		Display,
		Formatter,
		Result
	},
	os::unix::fs::{
		FileTypeExt,
		MetadataExt
	}
};
use users::get_user_by_uid;
use crate::error_handling::throw_error;

pub fn reveal_file(path: &PathBuf) {
	let file = File::open(path).unwrap_or_else(|_| {
		throw_error(
			"Could not open file.",
			"Ensure that you have enough permissions to read it."
		);
	});
	let reader = BufReader::new(file);
	for line in reader.lines() {
		let line = line.unwrap_or_else(|_| {
			throw_error(
				"Could not read lines of file.",
				"Ensure that the file is not a binary."
			);
		});
		eprintln!("{}", line);
	}
}

enum EntryKind {
	File,
	Directory,
	Socket,
	Character,
	Block,
	Fifo,
	Broken
}

impl EntryKind {
	fn from_file_type(file_type: &FileType) -> Self {
		if file_type.is_dir() {
			Self::Directory
		} else if file_type.is_file() {
			Self::File
		} else if file_type.is_socket() {
			Self::Socket
		} else if file_type.is_char_device() {
			Self::Character
		} else if file_type.is_block_device() {
			Self::Block
		} else if file_type.is_fifo() {
			Self::Fifo
		} else {
			Self::Broken
		}
	}
}

impl Display for EntryKind {
	fn fmt(&self, formatter: &mut Formatter) -> Result {
		let kind = match self {
			Self::File => {String::from("File")}
			Self::Directory => {String::from("Directory")}
			Self::Broken => {String::from("Broken")}
			Self::Fifo => {String::from("Fifo")}
			Self::Block => {String::from("Block")}
			Self::Socket => {String::from("Socket")}
			Self::Character => {String::from("Character")}
		};
		write!(formatter, "{}", kind)
	}
}

pub struct Permissions {
	mode: u32
}

const UNIX_OWNER_READING_PERMISSIONS_BIT: u32 = 0o400;
const UNIX_OWNER_WRITING_PERMISSIONS_BIT: u32 = 0o200;
const UNIX_OWNER_EXECUTION_PERMISSIONS_BIT: u32 = 0o100;
const UNIX_GROUP_READING_PERMISSIONS_BIT: u32 = 0o40;
const UNIX_GROUP_WRITING_PERMISSIONS_BIT: u32 = 0o20;
const UNIX_GROUP_EXECUTION_PERMISSIONS_BIT: u32 = 0o10;
const UNIX_OTHERS_READING_PERMISSIONS_BIT: u32 = 0o4;
const UNIX_OTHERS_WRITING_PERMISSIONS_BIT: u32 = 0o2;
const UNIX_OTHERS_EXECUTION_PERMISSIONS_BIT: u32 = 0o1;

impl Permissions {
	pub fn from_mode(mode: u32) -> Self {
		Self {mode}
	}

	fn does_owner_can_read(&self) -> bool {
		self.mode & UNIX_OWNER_READING_PERMISSIONS_BIT != 0
	}

	fn does_owner_can_write(&self) -> bool {
		self.mode & UNIX_OWNER_WRITING_PERMISSIONS_BIT != 0
	}

	fn does_owner_can_execute(&self) -> bool {
		self.mode & UNIX_OWNER_EXECUTION_PERMISSIONS_BIT != 0
	}

	fn does_group_can_read(&self) -> bool {
		self.mode & UNIX_GROUP_READING_PERMISSIONS_BIT != 0
	}

	fn does_group_can_write(&self) -> bool {
		self.mode & UNIX_GROUP_WRITING_PERMISSIONS_BIT != 0
	}

	fn does_group_can_execute(&self) -> bool {
		self.mode & UNIX_GROUP_EXECUTION_PERMISSIONS_BIT != 0
	}

	fn does_others_can_read(&self) -> bool {
		self.mode & UNIX_OTHERS_READING_PERMISSIONS_BIT != 0
	}

	fn does_others_can_write(&self) -> bool {
		self.mode & UNIX_OTHERS_WRITING_PERMISSIONS_BIT != 0
	}

	fn does_others_can_execute(&self) -> bool {
		self.mode & UNIX_OTHERS_EXECUTION_PERMISSIONS_BIT != 0
	}

	pub fn as_bits_sum(&self) -> u32 {
		let mut sum: u32 = 0;
		if self.does_owner_can_read() {
			sum += UNIX_OWNER_READING_PERMISSIONS_BIT;
		}
		if self.does_owner_can_write() {
			sum += UNIX_OWNER_WRITING_PERMISSIONS_BIT;
		}
		if self.does_owner_can_execute() {
			sum += UNIX_OWNER_EXECUTION_PERMISSIONS_BIT;
		}
		if self.does_group_can_read() {
			sum += UNIX_GROUP_READING_PERMISSIONS_BIT;
		}
		if self.does_group_can_write() {
			sum += UNIX_GROUP_WRITING_PERMISSIONS_BIT;
		}
		if self.does_group_can_execute() {
			sum += UNIX_GROUP_EXECUTION_PERMISSIONS_BIT;
		}
		if self.does_others_can_read() {
			sum += UNIX_OTHERS_READING_PERMISSIONS_BIT;
		}
		if self.does_others_can_write() {
			sum += UNIX_OTHERS_WRITING_PERMISSIONS_BIT;
		}
		if self.does_others_can_execute() {
			sum += UNIX_OTHERS_EXECUTION_PERMISSIONS_BIT;
		}
		sum
	}

	pub fn as_string(&self) -> String {
		const READING_CHARACTER: char = 'r';
		const WRITING_CHARACTER: char = 'w';
		const EXECUTION_CHARACTER: char = 'x';
		const NONE_CHARACTER: char = '-';
		let mut string: String = String::new();
		if self.does_owner_can_read() {
			string.push(READING_CHARACTER);
		} else {
			string.push(NONE_CHARACTER);
		}
		if self.does_owner_can_write() {
			string.push(WRITING_CHARACTER);
		} else {
			string.push(NONE_CHARACTER);
		}
		if self.does_owner_can_execute() {
			string.push(EXECUTION_CHARACTER);
		} else {
			string.push(NONE_CHARACTER);
		}
		if self.does_group_can_read() {
			string.push(READING_CHARACTER);
		} else {
			string.push(NONE_CHARACTER);
		}
		if self.does_group_can_write() {
			string.push(WRITING_CHARACTER);
		} else {
			string.push(NONE_CHARACTER);
		}
		if self.does_group_can_execute() {
			string.push(EXECUTION_CHARACTER);
		} else {
			string.push(NONE_CHARACTER);
		}
		if self.does_others_can_read() {
			string.push(READING_CHARACTER);
		} else {
			string.push(NONE_CHARACTER);
		}
		if self.does_others_can_write() {
			string.push(WRITING_CHARACTER);
		} else {
			string.push(NONE_CHARACTER);
		}
		if self.does_others_can_execute() {
			string.push(EXECUTION_CHARACTER);
		} else {
			string.push(NONE_CHARACTER);
		}
		string
	}
}

struct Entry {
	name: String,
	kind: EntryKind,
	size: u64,
	owner: Option<String>,
	symlink_path: Option<String>,
	permissions: Permissions
}

fn convert_size_to_human_readable_format(size: u64) -> String {
	if size == 0 {
		return String::from("-")
	}
	const ONE_GIGABYTE_AS_BYTES: u64 = 10_u64.pow(9);
	const ONE_MEGABYTE_AS_BYTES: u64 = 10_u64.pow(6);
	const ONE_KILOBYTE_AS_BYTES: u64 = 10_u64.pow(3);
	let gigabytes = size as f32 / ONE_GIGABYTE_AS_BYTES as f32;
	let megabytes = size as f32 / ONE_MEGABYTE_AS_BYTES as f32;
	let kilobytes = size as f32 / ONE_KILOBYTE_AS_BYTES as f32;
	if gigabytes as u64 > 0 {
		format!(
			"{:.1}GB",
			gigabytes
		)
	} else if megabytes as u64 > 0 {
		format!(
			"{:.1}MB",
			megabytes
		)
	} else if kilobytes as u64 > 0 {
		format!(
			"{:.1}kB",
			kilobytes
		)
	} else {
		format!(
			"{}B",
			size
		)
	}
}

pub fn reveal_directory(path: &PathBuf) {
	let stream = read_dir(path).unwrap_or_else(|_| {
		throw_error(
			"Could not open directory.",
			"Ensure that you have enough permissions to read it."
		);
	});
	let mut entries: Vec<Entry> = Vec::new();
	for entry in stream {
		let entry = entry.unwrap_or_else(|_| {
			throw_error(
				"Could not read entries of directory.",
				"Ensure that you have enough permissions to read it."
			);
		});
		let name = match entry.file_name().to_str() {
			Some(name) => {String::from(name)}
			None => {format!("{}", entry.path().display())}
		};
		let symlink_path = match read_link(entry.path()) {
			Ok(symlink_path) => {Some(format!("{}", symlink_path.display()))}
			Err(..) => {None}
		};
		let metadata = match entry.path().metadata() {
			Ok(metadata) => {metadata}
			Err(..) => {
				entries.push(Entry {
					name,
					kind: EntryKind::Broken,
					size: 0,
					owner: None,
					symlink_path,
					permissions: Permissions::from_mode(0)
				});
				continue;
			}
		};
		let permissions = Permissions::from_mode(metadata.mode());
		let file_type = metadata.file_type();
		let kind = EntryKind::from_file_type(&file_type);
		let size = if let EntryKind::File = kind {
			metadata.size()
		} else {
			0
		};
		let owner_uid = metadata.uid();
		let owner = match get_user_by_uid(owner_uid) {
			Some(user) => {
				match user.name().to_str() {
					Some(owner) => {Some(String::from(owner))}
					None => {None}
				}
			}
			None => {None}
		};
		entries.push(Entry {
			name,
			kind,
			size,
			owner,
			symlink_path,
			permissions
		});
	}
	entries.sort_by_key(|entry| {entry.name.clone()});
	println!("Owner       Size     Permissions    Type       Name");
	for entry in entries {
		println!(
			"{:<10}  {:<7}  {} {:o}  {:<9}  {}{}",
			match entry.owner {
				Some(owner) => {owner}
				None => String::from("-")
			},
			convert_size_to_human_readable_format(entry.size),
			entry.permissions.as_string(),
			entry.permissions.as_bits_sum(),
			format!("{}", entry.kind),
			entry.name,
			match entry.symlink_path {
				Some(symlink_path) => {format!(" => {}", symlink_path)}
				None => {String::new()}
			}
		);
	}
	println!("Path: {}.", format!("{}", path.display()));
}

