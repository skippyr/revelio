use std::{
	process::exit,
	env::args,
	path::PathBuf,
	fs::{
		File,
		FileType,
		read_dir,
		read_link
	},
	io::{
		BufReader,
		BufRead
	},
	os::unix::fs::{
		FileTypeExt,
		MetadataExt
	}
};
use users::get_user_by_uid;

fn throw_error(description: &str, suggestion: &str) -> ! {
	eprintln!("Error");
	eprintln!("\tDescription\n\t\t{}", description);
	eprintln!("\tSuggestion\n\t\t{}", suggestion);
	exit(1);
}

fn print_help_instructions() -> ! {
	println!("Reveal - Help Instructions");
	println!("\tStarting Point");
	println!("\t\tA program to reveal directory entries and file contents.");
	println!("\tSyntax");
	println!("\t\treveal [flags] <path>");
	println!("\tFlags");
	println!("\t\t--help: print these help instructions.");
	exit(0);
}

fn reveal_file(path: &PathBuf) {
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

struct Entry {
	name: String,
	kind: EntryKind,
	size: u64,
	owner: Option<String>,
	symlink_path: Option<String>
}

fn reveal_directory(path: &PathBuf) {
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
					symlink_path
				});
				continue;
			}
		};
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
			symlink_path
		});
	}
}

fn main() {
	let arguments = args().collect::<Vec<String>>();
	if arguments.contains(&String::from("--help")) {
		print_help_instructions();
	}
	if arguments.len() < 2 {
		throw_error(
			"Not enough arguments.",
			"Use the --help flag to see what arguments it can receive."
		);
	}
	let path = PathBuf::from(&arguments[1]).canonicalize().unwrap_or_else(|_| {
		throw_error(
			"The given path does not exists.",
			"Ensure that you did not mispell it."
		);
	});
	let metadata = path.metadata().unwrap_or_else(|_| {
		throw_error(
			"Can not get metadata of given path.",
			"Ensure that it is not a broken file."
		);
	});
	if metadata.is_file() {
		reveal_file(&path);
	} else if metadata.is_dir() {
		reveal_directory(&path);
	} else {
		throw_error(
			"Can not reveal this type.",
			"Use the --help flag to see what types can be revealed."
		);
	}
}
