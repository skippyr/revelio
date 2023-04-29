use std::{
	env::args,
	process::exit,
	fs::{
		canonicalize,
		Metadata,
		read_dir,
		ReadDir,
		DirEntry,
		FileType,
		Permissions
	},
	path::PathBuf,
	os::unix::prelude::{
		MetadataExt,
		FileTypeExt,
		PermissionsExt
	}
};

struct DirectoryEntry
{
	name: String,
	size_in_bytes: u64,
	file_type: FileType,
	permissions: Permissions
}

fn print_error(message: &str)
{
	eprintln!("Opsie!");
	eprintln!(
		"\treveal: {}",
		message
	);
	eprintln!("\tUse the -h or --help flags to print help instructions.");
	return;
}

fn print_help_instructions()
{
	eprintln!("Help Instructions");
	eprintln!("\tStarting Point");
	eprintln!("\t\tReveal is an utility tool to reveal directory entries and file contents.");
	eprintln!("\tSyntax");
	eprintln!("\t\tUse it with the following syntax:");
	eprintln!("\t\t\treveal <path>");
	eprintln!("\t\tIf no path is given, the current directory one will be considered.");
	eprintln!("\t\tIf multiple paths are given, only the last one will be revealed.");
	return;
}

fn covert_file_type_to_human_readable_string(file_type: &FileType) -> String
{
	let mut human_readable_string: String = String::from("Unknown");
	if file_type.is_file()
	{
		human_readable_string = String::from("File");
	}
	else if file_type.is_symlink()
	{
		human_readable_string = String::from("Symlink");
	}
	else if file_type.is_dir()
	{
		human_readable_string = String::from("Directory");
	}
	else if file_type.is_char_device()
	{
		human_readable_string = String::from("Character");
	}
	else if file_type.is_socket()
	{
		human_readable_string = String::from("Socket");
	}
	else if file_type.is_block_device()
	{
		human_readable_string = String::from("Block");
	}
	return human_readable_string;
}

fn convert_size_in_bytes_to_human_readable_string(size_in_bytes: u64) -> String
{
	let size_in_gigabytes: f32 = size_in_bytes as f32 / 10_f32.powi(9);
	let size_in_megabytes: f32 = size_in_bytes as f32 / 10_f32.powi(6);
	let size_in_kilobytes: f32 = size_in_bytes as f32 / 10_f32.powi(3);
	let mut human_readable_string: String = String::from("-");
	if size_in_gigabytes as u32 > 0
	{
		human_readable_string = format!(
			"{:.1}GB",
			size_in_gigabytes
		);
	}
	else if size_in_megabytes as u32 > 0
	{
		human_readable_string = format!(
			"{:.1}MB",
			size_in_megabytes
		);
	}
	else if size_in_kilobytes as u32 > 0
	{
		human_readable_string = format!(
			"{:.1}KB",
			size_in_kilobytes
		);
	}
	else if size_in_bytes > 0
	{
		human_readable_string = format!(
			"{}B",
			size_in_bytes
		);
	}
	return human_readable_string;
}

fn does_the_owner_has_reading_permissions(permissions_mode: u32) -> bool
{
	return permissions_mode & 0o400 != 0;
}

fn does_the_owner_has_writting_permissions(permissions_mode: u32) -> bool
{
	return permissions_mode & 0o200 != 0;
}

fn does_the_owner_has_execution_permissions(permissions_mode: u32) -> bool
{
	return permissions_mode & 0o100 != 0;
}

fn does_the_group_has_reading_permissions(permissions_mode: u32) -> bool
{
	return permissions_mode & 0o40 != 0;
}

fn does_the_group_has_writting_permissions(permissions_mode: u32) -> bool
{
	return permissions_mode & 0o20 != 0
}

fn does_the_group_has_execution_permissions(permissions_mode: u32) -> bool
{
	return permissions_mode & 0o10 != 0;
}

fn do_others_have_reading_permissions(permissions_mode: u32) -> bool
{
	return permissions_mode & 0o4 != 0;
}

fn do_others_have_writting_permissions(permissions_mode: u32) -> bool
{
	return permissions_mode & 0o2 != 0;
}

fn do_others_have_execution_permissions(permissions_mode: u32) -> bool
{
	return permissions_mode & 0o1 != 0;
}

fn convert_permissions_to_human_readable_string(permissions: &Permissions) -> String
{
	let mut human_readable_string: String = String::new();
	let mut bit_permissions_sum: u32 = 0;
	let permissions_mode: u32 = permissions.mode();
	const OWNER_READING_PERMISSION_BIT: u32 = 400;
	const OWNER_WRITTING_PERMISSIONS_BIT: u32 = 200;
	const OWNER_EXECUTION_PERMISSIONS_BIT: u32 = 100;
	const GROUP_READING_PERMISSIONS_BIT: u32 = 40;
	const GROUP_WRITTING_PERMISSIONS_BIT: u32 = 20;
	const GROUP_EXECUTION_PERMISSIONS_BIT: u32 = 10;
	const OTHERS_READING_PERMISSIONS_BIT: u32 = 4;
	const OTHERS_WRITTING_PERMISSIONS_BIT: u32 = 2;
	const OTHERS_EXECUTION_PERMISSIONS_BIT: u32 = 1;
	if does_the_owner_has_reading_permissions(permissions_mode)
	{
		human_readable_string.push('r');
		bit_permissions_sum += OWNER_READING_PERMISSION_BIT;
	}
	else
	{
		human_readable_string.push('-');
	}
	if does_the_owner_has_writting_permissions(permissions_mode)
	{
		human_readable_string.push('w');
		bit_permissions_sum += OWNER_WRITTING_PERMISSIONS_BIT;
	}
	else
	{
		human_readable_string.push('-');
	}
	if does_the_owner_has_execution_permissions(permissions_mode)
	{
		human_readable_string.push('x');
		bit_permissions_sum += OWNER_EXECUTION_PERMISSIONS_BIT;
	}
	else
	{
		human_readable_string.push('-');
	}
	if does_the_group_has_reading_permissions(permissions_mode)
	{
		human_readable_string.push('r');
		bit_permissions_sum += GROUP_READING_PERMISSIONS_BIT;
	}
	else
	{
		human_readable_string.push('-');
	}
	if does_the_group_has_writting_permissions(permissions_mode)
	{
		human_readable_string.push('w');
		bit_permissions_sum += GROUP_WRITTING_PERMISSIONS_BIT;
	}
	else
	{
		human_readable_string.push('-');
	}
	if does_the_group_has_execution_permissions(permissions_mode)
	{
		human_readable_string.push('x');
		bit_permissions_sum += GROUP_EXECUTION_PERMISSIONS_BIT;
	}
	else
	{
		human_readable_string.push('-');
	}
	if do_others_have_reading_permissions(permissions_mode)
	{
		human_readable_string.push('r');
		bit_permissions_sum += OTHERS_READING_PERMISSIONS_BIT;
	}
	else
	{
		human_readable_string.push('-');
	}
	if do_others_have_writting_permissions(permissions_mode)
	{
		human_readable_string.push('w');
		bit_permissions_sum += OTHERS_WRITTING_PERMISSIONS_BIT;
	}
	else
	{
		human_readable_string.push('-');
	}
	if do_others_have_execution_permissions(permissions_mode)
	{
		human_readable_string.push('x');
		bit_permissions_sum += OTHERS_EXECUTION_PERMISSIONS_BIT;
	}
	else
	{
		human_readable_string.push('-');
	}
	human_readable_string.push_str(&format!(
		" ({})",
		bit_permissions_sum
	));
	return human_readable_string;
}

fn reveal_directory(directory_path: &PathBuf)
{
	let directory_stream: ReadDir = match read_dir(directory_path)
	{
		Ok(directory_stream) =>
		{
			directory_stream
		}
		Err(_) =>
		{
			print_error("Could not read directory.");
			exit(1);
		}
	};
	let mut directory_entries: Vec<DirectoryEntry> = Vec::new();
	eprintln!(
		"Revealing directory: {:?}.",
		directory_path
	);
	for directory_entry in directory_stream
	{
		let directory_entry: DirEntry = match directory_entry
		{
			Ok(directory_entry) =>
			{
				directory_entry
			}
			Err(_) =>
			{
				continue;
			}
		};
		let directory_entry_path: PathBuf = directory_entry.path();
		let directory_entry_name_as_osstr = match directory_entry_path.file_name()
		{
			Some(directory_entry_name_as_osstr) =>
			{
				directory_entry_name_as_osstr
			}
			None =>
			{
				continue;
			}
		};
		let directory_entry_name: String = match directory_entry_name_as_osstr.to_str()
		{
			Some(directory_entry_name)=>
			{
				String::from(directory_entry_name)
			}
			None =>
			{
				continue;
			}
		};
		let directory_entry_metadata: Metadata = match directory_entry_path.metadata() {
			Ok(directory_entry_metadata) =>
			{
				directory_entry_metadata
			}
			Err(_) =>
			{
				continue;
			}
		};
		let directory_entry_size_in_bytes: u64 = if directory_entry_metadata.is_file()
		{
			directory_entry_metadata.size()
		}
		else
		{
			0
		};
		let directory_entry_file_type: FileType = directory_entry_metadata.file_type();
		let directory_entry_permissions: Permissions = directory_entry_metadata.permissions();
		directory_entries.push(DirectoryEntry {
			name: directory_entry_name,
			size_in_bytes: directory_entry_size_in_bytes,
			file_type: directory_entry_file_type,
			permissions: directory_entry_permissions
		});
	}
	for directory_entry_iterator in 0..directory_entries.len()
	{
		println!(
			"{:>4} | {:<9}   {:>8}   {}   {}",
			directory_entry_iterator + 1,
			covert_file_type_to_human_readable_string(&directory_entries[directory_entry_iterator].file_type),
			convert_size_in_bytes_to_human_readable_string(directory_entries[directory_entry_iterator].size_in_bytes),
			convert_permissions_to_human_readable_string(&directory_entries[directory_entry_iterator].permissions),
			directory_entries[directory_entry_iterator].name
		);
	}
	return;
}

fn main()
{
	let arguments: Vec<String> = args().collect();
	let mut relative_path: String = String::from(".");
	for arguments_iterator in 1..arguments.len()
	{
		if
			arguments[arguments_iterator] == "-h" ||
			arguments[arguments_iterator] == "--help"
		{
			print_help_instructions();
			exit(0);
		}
		else
		{
			relative_path = arguments[arguments_iterator].clone();
		}
	}
	let absolute_path: PathBuf = match canonicalize(&relative_path)
	{
		Ok(absolute_path) =>
		{
			absolute_path
		}
		Err(_) =>
		{
			print_error("Could not find given path.");
			exit(1);
		}
	};
	let absolute_path_metadata: Metadata = match absolute_path.metadata()
	{
		Ok(absolute_path_metadata) =>
		{
			absolute_path_metadata
		}
		Err(_) =>
		{
			print_error("Could not get metadata of given path.");
			exit(1);
		}
	};
	if absolute_path_metadata.is_dir()
	{
		reveal_directory(&absolute_path);
	}
	else
	{
		print_error("Could not reveal file type of given path.");
		exit(1);
	}
	return;
}

