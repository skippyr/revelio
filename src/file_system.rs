use std::
{
	fs::
	{
		self,
		ReadDir,
		read_dir,
		DirEntry,
		Metadata,
		read_link,
		FileType
	},
	ffi::OsStr,
	path::PathBuf,
	process::exit,
	io::
	{
		BufReader,
		BufRead
	},
	os::unix::prelude::
	{
		FileTypeExt,
		PermissionsExt
	}
};
use super::pretty_printing::print_error;

pub struct File
{ path: PathBuf }

impl File
{
	pub fn from(path: &PathBuf) -> File
	{ File { path: path.clone() } }

	pub fn reveal(&self)
	{
		let file: fs::File = fs::File::open(&self.path).unwrap_or_else(
			|_error|
			{
				print_error(String::from("Could not open file."));
				exit(1);
			}
		);
		let buffer: BufReader<fs::File> = BufReader::new(file);
		let mut line_number: u32 = 1;
		for line in buffer.lines()
		{
			let line: String = line.unwrap_or_else(
				|_error|
				{
					print_error(String::from("Could not read lines of file."));
					exit(1);
				}
			);
			println!(
				"{:>5} | {}",
				line_number,
				line
			);
			line_number += 1;
		}
	}
}

#[derive(Debug)]
enum DirectoryEntryKind
{
	File,
	Directory,
	Socket,
	Character,
	Block,
	Fifo
}

impl DirectoryEntryKind
{
	pub fn from(file_type: &FileType) -> DirectoryEntryKind
	{
		if file_type.is_file()
		{ DirectoryEntryKind::File }
		else if file_type.is_dir()
		{ DirectoryEntryKind::Directory }
		else if file_type.is_socket()
		{ DirectoryEntryKind::Socket }
		else if file_type.is_char_device()
		{ DirectoryEntryKind::Character }
		else if file_type.is_block_device()
		{ DirectoryEntryKind::Block }
		else
		{ DirectoryEntryKind::Fifo }
	}

	pub fn as_string(&self) -> String
	{
		match self
		{
			DirectoryEntryKind::File =>
			{ String::from("File") }
			DirectoryEntryKind::Directory =>
			{ String::from("Directory") }
			DirectoryEntryKind::Socket =>
			{ String::from("Socket") }
			DirectoryEntryKind::Character =>
			{ String::from("Character") }
			DirectoryEntryKind::Block =>
			{ String::from("Block") }
			DirectoryEntryKind::Fifo =>
			{ String::from("Fifo") }
		}
	}
}

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
	pub fn new(permissions_mode: u32) -> UnixPermissions
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

pub struct DirectoryEntry
{
	name: String,
	kind: DirectoryEntryKind,
	symlink_path: Option<PathBuf>,
	permissions: UnixPermissions
}

impl DirectoryEntry
{
	pub fn new(
		name: String,
		file_type: FileType,
		symlink_path: Option<PathBuf>,
		permissions_mode: u32
	) -> DirectoryEntry
	{
		DirectoryEntry
		{
			name,
			kind: DirectoryEntryKind::from(&file_type),
			symlink_path,
			permissions: UnixPermissions::new(permissions_mode)
		}
	}

	pub fn as_string(&self) -> String
	{
		format!(
			"{}{:<10}  {} ({:o})   {}",
			match &self.symlink_path
			{
				Some(_symlink_path) =>
				{ String::from("@") }
				None =>
				{ String::from(" ") }
			},
			self.kind.as_string(),
			self.permissions.as_string(),
			self.permissions.as_bits_sum(),
			self.name
		)
	}
}

pub struct Directory
{ path: PathBuf }

impl Directory
{
	pub fn from(path: &PathBuf) -> Directory
	{ Directory { path: path.clone() } }

	pub fn get_entries(&self) -> Vec<DirectoryEntry>
	{
		let stream: ReadDir = read_dir(&self.path).unwrap_or_else(
			|_error|
			{
				print_error(String::from("Could not read directory."));
				exit(1);
			}
		);
		let mut entries: Vec<DirectoryEntry> = Vec::new();
		for entry in stream
		{
			let entry: DirEntry = match entry
			{
				Ok(entry) =>
				{ entry }
				Err(_error) =>
				{ continue; }
			};
			let path: PathBuf = entry.path();
			let file_name: &OsStr = match path.file_name()
			{
				Some(file_name) =>
				{ file_name }
				None =>
				{ continue; }
			};
			let name: String = match file_name.to_str()
			{
				Some(name) =>
				{ String::from(name) }
				None =>
				{ continue; }
			};
			let metadata: Metadata = match entry.metadata()
			{
				Ok(metadata) =>
				{ metadata }
				Err(_error) =>
				{ continue; }
			};
			let file_type: FileType = metadata.file_type();
			let symlink_path: Option<PathBuf> = match read_link(&path)
			{
				Ok(symlink_path) =>
				{ Some(symlink_path) }
				Err(_error) =>
				{ None }
			};
			let permissions_mode: u32 = metadata.permissions().mode();
			entries.push(DirectoryEntry::new(
				name,
				file_type,
				symlink_path,
				permissions_mode
			));
		}
		entries.sort_by_key(
			|entry|
			entry.name.clone()
		);
		entries
	}

	pub fn reveal(&self)
	{
		let mut entry_number: u32 = 1;
		for entry in self.get_entries()
		{
			eprintln!(
				"{:>5} | {}",
				entry_number,
				entry.as_string()
			);
			entry_number += 1;
		}
	}
}

