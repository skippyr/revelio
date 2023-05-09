use std::
{
	fs::
	{
		read_dir,
		ReadDir,
		DirEntry,
		Metadata,
		FileType
	},
	path::PathBuf,
	ffi::OsStr,
	os::unix::fs::
	{
		PermissionsExt,
		FileTypeExt
	}
};
use crate::
{
	errors::Error,
	file_system::permissions::UnixPermissions
};

enum DirectoryEntryKind
{
	File,
	Directory,
	Socket,
	Character,
	Block,
	Fifo,
	Unknown
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
		else if file_type.is_fifo()
		{ DirectoryEntryKind::Fifo }
		else
		{ DirectoryEntryKind::Unknown }
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
			DirectoryEntryKind::Unknown =>
			{ String::from("Unknown") }
		}
	}
}

struct DirectoryEntry
{
	name: String,
	permissions: UnixPermissions,
	kind: DirectoryEntryKind
}

impl DirectoryEntry
{
	pub fn as_string(&self) -> String
	{
		format!(
			"{:<9}   {} ({:o})   {}",
			self.kind.as_string(),
			self.permissions.as_string(),
			self.permissions.as_bits_sum(),
			self.name
		)
	}
}

pub struct Directory
{ stream: ReadDir }

impl Directory
{
	pub fn from(path: &PathBuf) -> Directory
	{ Directory { stream: Directory::get_stream(path) } }

	fn get_stream(path: &PathBuf) -> ReadDir
	{
		match read_dir(path)
		{
			Ok(stream) =>
			{ stream }
			Err(_error) =>
			{
				Error::new(
					String::from("could not read directory."),
					String::from("ensure that you have enough permissions to read it."),
					1
				).throw();
			}
		}
	}

	fn get_entries(&mut self) -> Vec<DirectoryEntry>
	{
		let mut entries: Vec<DirectoryEntry> = Vec::new();
		for entry in self.stream.by_ref()
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
			let metadata: Metadata = match path.metadata()
			{
				Ok(metadata) =>
				{ metadata }
				Err(_error) =>
				{ continue; }
			};
			let file_type: FileType = metadata.file_type();
			let permissions_mode: u32 = metadata.permissions().mode();
			entries.push(
				DirectoryEntry
				{
					name,
					permissions: UnixPermissions::from(permissions_mode),
					kind: DirectoryEntryKind::from(&file_type)
				}
			)
		}
		entries
	}

	pub fn reveal(&mut self)
	{
		let entries: Vec<DirectoryEntry> = self.get_entries();
		let mut entry_number: u32 = 0;
		for entry in entries
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

