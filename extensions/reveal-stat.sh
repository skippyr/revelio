# Reveals the metadata of entries similarly to the stat command.
#
# It expects a list of paths as arguments.
function reveal-stat {
  typeset IFS=$'\n'
  for path_ in $@; do
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
    echo -e "Path: \"${path_}\".  Size: ${metadata[1]}.  Inode: ${metadata[2]}.\nPermissions: (${metadata[3]}/${metadata[4]}).  Owner: (${metadata[5]}/${metadata[6]}).  Group: (${metadata[7]}/${metadata[8]}).\nModified Date: ${metadata[9]}.\n"
  done
}
