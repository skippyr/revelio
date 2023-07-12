file=reveal
optimizationLevel=z

${file}:
	clang ${file}.c -o ${file} -O${optimizationLevel}

.PHONY: ${file}

