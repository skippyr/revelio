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
	os::unix::
	{
		fs::
		{
			PermissionsExt,
			FileTypeExt
		},
		prelude::MetadataExt
	}
};
use crate::
{
	errors::Error,
	users::UnixUser,
	locale::NumberFormatter,
	file_system::
	{
		permissions::UnixPermissions,
		symlink::Symlink,
		sizes::DigitalSize
	}
};

enum DirectoryEntryKind
{
	File,
	Directory,
	Socket,
	Character,
	Block,
	Fifo,
	Broken
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
		{ DirectoryEntryKind::Broken }
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
			DirectoryEntryKind::Broken =>
			{ String::from("Broken") }
		}
	}
}

struct DirectoryEntry
{
	name: String,
	permissions: UnixPermissions,
	kind: DirectoryEntryKind,
	size: DigitalSize,
	owner: UnixUser,
	symlink: Symlink
}

impl DirectoryEntry
{
	pub fn as_string(&self) -> String
	{
		format!(
			"{}{:<9}   {:>7}   {} ({:o})   {:<10}   {}{}",
			self.symlink.as_decorator_string(),
			self.kind.as_string(),
			self.size.as_string(),
			self.permissions.as_string(),
			self.permissions.as_bits_sum(),
			self.owner.get_name(),
			self.name,
			self.symlink.as_string()
		)
	}
}

pub struct Directory
{
	path: PathBuf,
	stream: ReadDir
}

impl Directory
{
	pub fn from(path: &PathBuf) -> Directory
	{
		Directory
		{
			path: path.clone(),
			stream: Directory::get_stream(path)
		}
	}

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
					String::from("ensure that you have enough permissions to read it.")
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
			let mut name: String = format!(
				"{}",
				path.display()
			);
			if let Some(file_name) = path.file_name()
			{
				if let Some(file_name_as_str) = file_name.to_str()
				{ name = String::from(file_name_as_str); }
			}
			let metadata: Metadata = match path.metadata()
			{
				Ok(metadata) =>
				{ metadata }
				Err(_error) =>
				{
					match entry.metadata()
					{
						Ok(metadata) =>
						{ metadata }
						Err(_error) =>
						{ continue; }
					}
				}
			};
			let kind: DirectoryEntryKind = DirectoryEntryKind::from(&metadata.file_type());
			let size: DigitalSize = DigitalSize::from(
				if metadata.is_file()
				{ metadata.size() }
				else
				{ 0 }
			);
			let owner: UnixUser = UnixUser::from(metadata.uid());
			let permissions: UnixPermissions = UnixPermissions::from(metadata.permissions().mode());
			let symlink: Symlink = Symlink::from(&path);
			entries.push(
				DirectoryEntry
				{
					name,
					permissions,
					kind,
					size,
					owner,
					symlink
				}
			)
		}
		entries.sort_by_key(
			|entry|
			{ entry.name.clone() }
		);
		entries
	}

	pub fn reveal(&mut self)
	{
		let entries: Vec<DirectoryEntry> = self.get_entries();
		let mut entry_number: u32 = 1;
		println!(
			"Revealing directory: {}.",
			self.path.display()
		);
		println!(" Index | Type            Size   Permissions       Owner        Name");
		for entry in entries
		{
			println!(
				"{:>6} | {}",
				NumberFormatter::format_u32(entry_number),
				entry.as_string()
			);
			entry_number += 1;
		}
	}
}

