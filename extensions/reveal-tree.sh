function reveal-tree {
  # Temporarily change the internal field separator to disconsider spaces in
  # paths' names.
  typeset IFS=$'\n'

  # Create a function that will reveal each entry of a directory in a tree view.
  function tree-view {
    typeset path_="$1"
    typeset depth="$2"

    for entry in $(reveal "${path_}"); do
      # Prints some decoration to help visualize the contents.
      printf "│  %.0s" {0..${depth}}
      printf "├──"

      # Prints the base name of the entry.
      echo ${entry##*/}

      # If the entry is a directory, it starts a recursion by executing the
      # same function on it again.
      [[ -d "${entry}" ]] &&
        $0 "${entry}" $((${depth} + 1))
    done
  }

  # Runs the function in the current directory.
  tree-view . 0

  # Unsets the function defined previously to avoid exposing it to the shell.
  unset -f tree-view
}
