# Reveals directories entries alphabetically. It is similar to the ls -A
# command one.
function reveal-ls {
  typeset IFS=$'\n'
  typeset paths=($@)
  [[ ${#paths[@]} -eq 0 ]] &&
    paths+=(".")
  for path_ in ${paths[@]}; do
    if [[ ! -d "${path_}" ]]; then
      echo -e "$0: \"${path_}\" is not a directory.\n"
      continue
    fi
    echo "${path_}:"
    for entry in $(reveal "${path_}" | sort); do
      echo "  ${entry##*/}"
    done
    echo
  done | fmt
}
