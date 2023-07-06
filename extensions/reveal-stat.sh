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
      --mode ${path_}\
      --type ${path_}\
      --permissions ${path_}\
      --human-permissions ${path_}\
      --user-id ${path_}\
      --user ${path_}\
      --group-id ${path_}\
      --group ${path_}\
      --modified-date ${path_}\
      --changed-date ${path_}\
      --accessed-date ${path_}))
    output+="Path: \"${path_}\".\nSize: ${metadata[1]}.  Inode: ${metadata[2]}.  Mode:${metadata[3]}.  Type: ${metadata[4]}.\nPermissions: (${metadata[5]}/${metadata[6]}).  UID: (${metadata[7]}/${metadata[8]}).  GID: (${metadata[9]}/${metadata[10]}).\nModified Date: ${metadata[11]}.\nChanged Date: ${metadata[12]}.\nAccessed Date: ${metadata[13]}.\n"
  done
  echo "${output}"
}
