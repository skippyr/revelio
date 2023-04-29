<h1>Reveal</h1>
	<h2>Starting Point</h2>
		<p>Reveal is an utility tool to reveal directory entries and file contents.</p>
		<img src="./images/preview.gif"/>
	<h2>Installation And Usage</h2>
		<p>Be aware that Reveal was built to run on Linux. I can not ensure that it will run in other systems as well.</p>
		<ul>
			<li>Install <a href="https://www.rust-lang.org/">Rust development tools</a>.</li>
			<li>Download this repository to a directory in your machine. If you have <code>git</code> installed, you can use it in the following command:</li>
			<pre><code>git clone --depth 1 https://github.com/skippyr/reveal</code></pre>
			<p>This command will clone this repository to the directory <code>reveal</code>, but feel free to change to whatever directory you want to, just remember its path because you will need it for the next step. The flag <code>--depth</code> with value <code>1</code> specifies to <code>git</code> that you only want to download the latest commit instead of the whole commit tree.</p>
			<p>If you do not have <code>git</code> installed, you can download this repository from its page on GitHub. Access that page, click on the <code>Code</code> button on the top of the page, then click on <code>Download ZIP</code>. This will download a ZIP file containing the repository, you just have to unzip it and move to the path you want it to be.</p>
			<li>Access the repository directory.</li>
			<li>Use <code>cargo</code> to build a release.</li>
			<pre><code>cargo build -r</code></pre>
			<p>The binary file will be available at <code>target/release/reveal</code>. You can run it using the <code>-h</code> or <code>--help</code> flag to print help instructions.</p>
			<pre><code>cd target/release</code></pre>
			<pre><code>./reveal -h</code></pre>
			<p>Alternatively, instead of running the binary directly, you can move it to a directory that is in your <code>PATH</code> variable. Like this, it will be available as any other system command.</p>
			<p>As an example, the commands below will move the binary to <code>~/.local/bin</code> and will add that directory to the <code>PATH</code> variable for your current shell session.</p>
			<pre><code>mkdir -p ~/.local/bin</code></pre>
			<pre><code>mv reveal ~/.local/bin</code></pre>
			<pre><code>export PATH="${PATH}:~/.local/bin"</code></pre>
			<p>If you want to make this change persistent to all shell sessions you will have, you can add that <code>export</code> rule to your shell's configuration file:</p>
			<ul>
				<li>If you are using Bash: <code>~/.bashrc</code>.</li>
				<li>If you are using ZSH: <code>~/.zshrc</code>.</li>
			</ul>
		</ul>
	<h2>Issues And Contributions</h2>
		<p>Learn how to report issues, questions and ideas and how to contribute to this project by reading its <a href="https://skippyr.github.io/materials/pages/contributions_guideline.html">contributions guideline</a>.</p>
	<h2>License</h2>
		<p>Reveal is released under the MIT License. You can refer to the license as the file <code><a href="https://github.com/skippyr/reveal/blob/main/LICENSE">LICENSE</a></code> in the root directory of this repository.</p>
		<p>Copyright (c) 2023, Sherman Rofeman. MIT License.</p>
