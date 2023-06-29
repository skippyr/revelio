# Reveals directories entries alphabetically and recursively using a tree view
# format. It is similar to the `tree` command.
#
# It will use the first argument given or, if no argument is given, it will
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
    for entry in $(reveal "${path_}" | sort); do
      printf "│  %.0s" {0..${depth}}
      printf "├──"
      echo ${entry##*/}
      [[ -d "${entry}" ]] &&
        $0 "${entry}" $((${depth} + 1))
    done
  }
  tree-view "${path_}" 0
  unset -f tree-view
}
