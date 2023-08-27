f=reveal

${f}:
	clang ${f}.c -o ${f} -Oz

.PHONY: ${f}
