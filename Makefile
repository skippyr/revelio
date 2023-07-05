file=reveal
optz=z

${file}:
	clang ${file}.c -o ${file} -O${optz}

.PHONY: ${file}

