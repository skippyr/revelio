# Reveals directories entries alphabetically and recursively using a tree view
# format. It reveals up to 5 levels of subdirectories not including symlinks
# that point to directories.
#
# It will consider the first argument given or, if no argument is given, it will
# consider the current directory.
function reveal-tree {
  typeset IFS=$'\n'
  typeset path_="$1"
  [[ "${path_}" == "" ]] &&
    path_="."
  if [[ ! -d "${path_}" ]]; then
    echo -e "$0: \"${path_}\" is not a directory.\n"
    return
  fi
  function tree-view {
    typeset path_="$1"
    typeset depth="$2"
    for entry in $(reveal --transpass "${path_}" | sort -f); do
      [[ ${depth} -gt 0 ]] &&
        printf "│  %.0s" {1..${depth}}
      printf "├──"
      echo ${entry##*/}
      [[ ! -L "${entry}" && -d "${entry}" && ${depth} -le 5 ]] &&
        $0 "${entry}" $((${depth} + 1))
    done
  }
  echo "${path_}"
  tree-view "${path_}" 0
  unset -f tree-view
}
