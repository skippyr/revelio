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
      --type ${path_}\
      --inode ${path_}\
      --mode ${path_}\
      --blocks ${path_}\
      --hard-links ${path_}\
      --permissions ${path_}\
      --human-permissions ${path_}\
      --user-id ${path_}\
      --user ${path_}\
      --group-id ${path_}\
      --group ${path_}\
      --modified-date ${path_}\
      --changed-date ${path_}\
      --accessed-date ${path_}))
          output+="Path: \"${path_}\".\nSize: ${metadata[1]}.  Type: ${metadata[2]}.\nInode: ${metadata[3]}.  Mode: ${metadata[4]}.  Blocks: ${metadata[5]}.  Hard Links: ${metadata[6]}.\nPermissions: (${metadata[7]}/${metadata[8]}).  UID: (${metadata[9]}/${metadata[10]}).  GID: (${metadata[11]}/${metadata[12]}).\nModified Date: ${metadata[13]}.\nChanged Date: ${metadata[14]}.\nAccessed Date: ${metadata[15]}.\n"
  done
  echo "${output}"
}
