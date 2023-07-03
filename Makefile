file=reveal
opt=z

${file}:
	clang ${file}.c -o ${file} -O${opt}

run: ${file}
	./${file}

.PHONY: ${file} run

