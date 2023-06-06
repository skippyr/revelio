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
	},
	fmt::{
		Result,
		Display,
		Formatter
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

struct Entry {
	name: String,
	kind: EntryKind,
	size: u64,
	owner: Option<String>,
	symlink_path: Option<String>
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
	entries.sort_by_key(|entry| {entry.name.clone()});
	for entry in entries {
		println!(
			"{:<10}  {:<7}  {:<9}  {}{}",
			match entry.owner {
				Some(owner) => {owner}
				None => String::from("-")
			},
			convert_size_to_human_readable_format(entry.size),
			format!("{}", entry.kind),
			entry.name,
			match entry.symlink_path {
				Some(symlink_path) => {format!(" => {}", symlink_path)}
				None => {String::new()}
			}
		);
	}
}

fn main() {
	let arguments = args().collect::<Vec<String>>();
	if arguments.contains(&String::from("--help")) {
		print_help_instructions();
	}
	let path = PathBuf::from(if arguments.len() < 2 {
		"."
	} else {
		&arguments[1]
	}).canonicalize().unwrap_or_else(|_| {
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
