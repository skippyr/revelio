use std::{
	env::args,
	process::exit,
	fs::{
		canonicalize,
		Metadata,
		read_dir,
		ReadDir,
		DirEntry
	},
	path::PathBuf,
	os::unix::prelude::MetadataExt
};

struct DirectoryEntry
{
	name: String,
	size_in_bytes: u64
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
		directory_entries.push(DirectoryEntry {
			name: directory_entry_name,
			size_in_bytes: directory_entry_size_in_bytes
		});
	}
	for directory_entry_iterator in 0..directory_entries.len()
	{
		println!(
			"{:>4} | {:>8}   {}",
			directory_entry_iterator + 1,
			convert_size_in_bytes_to_human_readable_string(directory_entries[directory_entry_iterator].size_in_bytes),
			directory_entries[directory_entry_iterator].name
		);
	}
	return;
}

fn main()
{
	let arguments: Vec<String> = args().collect();
	let mut relative_path: String = String::from(".");
	if arguments.len() == 1
	{
		print_error("Not enough arguments.");
		exit(1);
	}
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

