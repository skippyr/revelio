file=reveal
opt=z

${file}:
	clang ${file}.c -o ${file} -O${opt}

.PHONY: ${file} run

