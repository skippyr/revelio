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
      --type ${path_}\
      --permissions ${path_}\
      --human-permissions ${path_}\
      --user-id ${path_}\
      --user ${path_}\
      --group-id ${path_}\
      --group ${path_}\
      --modified-date ${path_}))
    output+="Path: \"${path_}\".  Size: ${metadata[1]}.  Inode: ${metadata[2]}.  Type: ${metadata[3]}.\nPermissions: (${metadata[4]}/${metadata[5]}).  UID: (${metadata[6]}/${metadata[7]}).  GID: (${metadata[8]}/${metadata[9]}).\nModified Date: ${metadata[10]}.\n"
  done
  echo "${output}"
}
