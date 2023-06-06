<h1>Reveal</h1>
	<h2>Starting Point</h2>
		<p>A program to reveal directory entries and file contents for UNIX-like operating systems. It works similarly to the <code>ls</code> and <code>cat</code> commands.</p>
		<img src="./images/preview.gif"/>
		<p>In the preview, reveal was used on the <a href="https://github.com/kovidgoyal/kitty">Kitty</a> terminal emulator with the <a href="https://github.com/skippyr/river_dreams">River Dreams</a> ZSH theme, <a href="https://github.com/skippyr/flamerial">Flamerial</a> theme and <a href="https://github.com/be5invis/Iosevka">Iosevka</a> font (stylistic set <code>ss08</code>).</p>
	<h2>Installation And Usage</h2>
		<ul>
			<li>Install the required dependencies:</li>
				<ul>
					<li>Install Rust toolchain.</li>
						<p>These are the tools needed to compile the source code.</p>
					<li>Install Git.</li>
						<p>This tool will be used to download the repository.</p>
				</ul>
			<li>Clone this repository.</li>
				<pre><code>git clone --depth=1 https://github.com/skippyr/reveal</code></pre>
			<li>Access the repository's directory.</li>
				<pre><code>cd reveal</code></pre>
			<li>Build a release.</li>
				<pre><code>cargo build --release</code></pre>
			<li>Link the binary file to a directory that is in your <code>${PATH}</code> variable.</li>
				<p>In this example, the directory <code>~/.local/bin</code> will be added to the <code>${PATH}</code> variable. Make this change persistent by adding the same export command to your shell startup.</p>
				<pre><code>
mkdir -p ~/.local/bin
export PATH="${PATH}:${HOME}/.local/bin"
ln -sf $(pwd)/target/release/reveal ~/.local/bin
				</code></pre>
			<li>Use the binary with the <code>--help</code> flag to see help instructions.</li>
				<pre><code>reveal --help</code></pre>
			<p>Give it a test by passing the path of a file or directory as an argument.</p>
		</ul>
	<h2>Issues And Contributions</h2>
		<p>Learn how to report issues, questions and ideas and how to contribute to this project by reading its <a href="https://skippyr.github.io/materials/pages/contributions_guidelines.html">contributions guidelines</a>.</p>
	<h2>License</h2>
		<p>This project is released under terms of the MIT License.</p>
		<p>Copyright (c) 2023, Sherman Rofeman. MIT License.</p>


