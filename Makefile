file=reveal
std=c++20

${file}:
	clang++ ${file}.cpp -std=${std} -o ${file}

run: ${file}
	./${file}

.PHONY: ${file} run

