function reveal-ls {
  # Temporarily change the internal field separator to disconsider spaces in
  # paths' names.
  typeset IFS=$'\n'

  # Collect all the paths given as parameters.
  typeset paths=($@)

  # If no path is given, consider the current directory.
  [[ ${#paths[@]} -eq 0 ]] &&
    paths+=(".")

  for path_ in ${paths[@]}; do
    # Checks if the path given is a directory, and prints an error if it is
    # not.
    if [[ ! -d "${path_}" ]]; then
      echo -e "$0: \"${path_}\" is not a directory.\n"
      continue
    fi

    # Prints a title to help visualize the contents.
    echo "${path_}:"

    # Loop through each entry of the directory and prints its base name.
    # Use sort to order it alphabetically.
    for entry in $(reveal "${path_}" | sort); do
      echo "  ${entry##*/}"
    done

    echo

    # In the end, pipe the output to fmt to format it and make it fit better
    # in the screen.
  done | fmt
}
