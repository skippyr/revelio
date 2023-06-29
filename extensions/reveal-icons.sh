# Reveals directories entries alphabetically with Nerd Font icons.
function reveal-icons {
  typeset IFS=$'\n'
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
    "ts"        "󰛦 "
    "css"       " "
    "scss"      " "
    "java"      "󰬷 "
    "cpp"       " "
    "hpp"       " "
    "c"         " "
    "h"         " "
    "cs"        " "
    "rs"        " "
    "tex"       " "
    "lua"       " "
    "py"        "󰌠 "
    "go"        " "
    "zip"       "󰛫 "
    "tar"       "󰛫 "
    "gz"        "󰛫 "
    "md"        " "
    "yml"       " "
    "json"      "󰘦 "
    "toml"      "󰬛 "
    "xcf"       "󰃣 "
    "conf"      " "
    "mk"        " "
    "ttf"       " "
    "otf"       " "
    "ufo"       " "
    "png"       "󰈟 "
    "jpg"       "󰈟 "
    "jpeg"      "󰈟 "
    "bmp"       "󰈟 "
    "iso"       "󱛟 "
    "diff"      "󰦓 "
    "info"      " "
    "o"         " "
    "out"       " "
    "1"         " "
    "2"         " "
    "3"         " "
    "4"         " "
    "5"         " "
    "6"         " "
    "7"         " "
    "8"         " "
  )
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
      typeset name="${entry##*/}"
      typeset extension="${entry##*.}"
      printf "  "
      if [[ -d "${entry}" ]]; then
        printf " "
      else
        typeset name_icon="${name_icons[${name}]}"
        typeset extension_icon="${extension_icons[${extension}]}"
        if [[ "${name_icon}" != "" ]]; then
          printf "${name_icon}"
        elif [[ "${extension_icon}" != "" ]]; then
          printf "${extension_icon}"
        else
          printf " "
        fi
      fi
      echo " ${name}"
    done
    echo
  done
}
