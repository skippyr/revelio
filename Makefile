file=reveal
optimization=z

${file}:
	clang ${file}.c -o ${file} -O${optimization}

.PHONY: ${file}
