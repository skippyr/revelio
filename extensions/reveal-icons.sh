# Reveals directories entries alphabetically with Nerd Font icons.
function reveal-icons {
  typeset IFS=$'\n'
  typeset paths=($@)
  function get-icon {
    typeset -r path_="$1"
    if [[ -d "${path_}" ]]; then
      echo " "
      return
    fi
    typeset -r name="$2"
    typeset -r extension="$3"
    typeset -rA name_icons=(
      ".clang-format" " "
      ".editorconfig" " "
      ".gitignore"    " "
      "LICENSE"       " "
      "Makefile"      " "
    )
    typeset -rA extension_icons=(
      "zsh"       " "
      "sh"        " "
      "zsh-theme" "󰏘 "
      "html"      " "
      "xml"       "󰗀 "
      "rb"        "󰴭 "
      "js"        "󰌞 "
      "css"       " "
      "scss"      " "
      "java"      "󰬷 "
      "cpp"       " "
      "hpp"       " "
      "c"         " "
      "h"         " "
      "cs"        " "
      "rs"        " "
      "md"        " "
      "xcf"       "󰃣 "
      "conf"      " "
      "mk"        " "
      "ttf"       " "
      "otf"       " "
      "ufo"       " "
      "png"       "󰈟 "
      "jpg"       "󰈟 "
      "bmp"       "󰈟 "
      "o"         " "
      "1"         " "
    )
    typeset -r name_icon="${name_icons[${name}]}"
    if [[ "${name_icon}" != "" ]]; then
      echo "${name_icon}"
      return
    fi
    typeset -r extension_icon="${extension_icons[${extension}]}"
    if [[ "${extension_icon}" != "" ]]; then
      echo "${extension_icon}"
      return
    fi
    echo " "
  }
  [[ ${#paths[@]} -eq 0 ]] &&
    paths+=(".")
  for path_ in ${paths[@]}; do
    if [[ ! -d "${path_}" ]]; then
      echo -e "$0: \"${path_}\" is not a directory.\n"
      continue
    fi
    echo "${path_}:"
    for entry in $(reveal "${path_}" | sort); do
      typeset name="${entry##*/}"
      typeset extension="${entry##*.}"
      typeset icon="$(get-icon "${entry}" "${name}" "${extension}")"
      echo "  ${icon} ${name}"
    done
    echo
  done
  unset -f get-icon
}
