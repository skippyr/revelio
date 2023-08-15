file=reveal
optimization_level=z

${file}:
	clang ${file}.c -o ${file} -O${optimization_level}

.PHONY: ${file}
