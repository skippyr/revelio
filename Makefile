f=reveal
o=z

${f}:
	clang ${f}.c -o ${f} -O${o}

.PHONY: ${f}
