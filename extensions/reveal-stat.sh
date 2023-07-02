# Reveals the metadata of entries similarly to the stat command.
#
# It will consider any path given as argument or, if no argument is provided,
# the current directory.
function reveal-stat {
  typeset IFS=$'\n'
  typeset paths=($@)
  [[ ${#paths[@]} -eq 0 ]] &&
    paths+=(".")
  typeset output=""
  for path_ in ${paths[@]}; do
    [[ -n "${output}" ]] &&
      output+="\n"
    if [[ ! -L "${path_}" && ! -e "${path_}" ]]; then
      output+="$0: the path \"${path_}\" does not exists.\n"
      continue
    fi
    typeset metadata=($(reveal\
      --human-size ${path_}\
      --inode ${path_}\
      --permissions ${path_}\
      --human-permissions ${path_}\
      --owner-uid ${path_}\
      --owner ${path_}\
      --group-uid ${path_}\
      --group ${path_}\
      --modified-date ${path_}))
    output+="Path: \"${path_}\".  Size: ${metadata[1]}.  Inode: ${metadata[2]}.\nPermissions: (${metadata[3]}/${metadata[4]}).  Owner: (${metadata[5]}/${metadata[6]}).  Group: (${metadata[7]}/${metadata[8]}).\nModified Date: ${metadata[9]}.\n"
  done
  echo "${output}"
}
