# Reveals directories recursively starting from the current directory using
# a tree view. It is similar to the `tree` command.
function reveal-tree {
  typeset IFS=$'\n'
  function tree-view {
    typeset path_="$1"
    typeset depth="$2"
    for entry in $(reveal "${path_}"); do
      printf "│  %.0s" {0..${depth}}
      printf "├──"
      echo ${entry##*/}
      [[ -d "${entry}" ]] &&
        $0 "${entry}" $((${depth} + 1))
    done
  }
  tree-view . 0
  unset -f tree-view
}
