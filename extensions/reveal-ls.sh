# Reveals directories entries alphabetically using the ls command format
#
# It will consider any directory path given as argument or, if no argument is
# provided, the current directory.
function reveal-ls {
  typeset IFS=$'\n'
  typeset paths=($@)
  [[ ${#paths[@]} -eq 0 ]] &&
    paths+=(".")
  typeset output=""
  for path_ in ${paths[@]}; do
    [[ -n "${output}" ]] &&
      output+="\n"
    if [[ ! -d "${path_}" ]]; then
      output+="$0: \"${path_}\" is not a directory.\n"
      continue
    fi
    output+="${path_}:\n"
    for entry in $(reveal --transpass "${path_}" | sort); do
      typeset base_name="${entry##*/}"
      output+="  "
      [[ "${base_name}" =~ " " ]] &&
        output+="'${base_name}'" ||
        output+="${base_name}"
        output+="\n"
    done
  done
  echo "${output}" | fmt
}
