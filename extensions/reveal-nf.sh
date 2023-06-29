# Reveals directories entries alphabetically and using Nerd Font icons.
#
# It will consider any directory path given as argument or, if no argument is
# provided, the current directory.
#
# It requires the use of font patched the Nerd Fonts project. You can download
# one here:
#
# https://github.com/ryanoasis/nerd-fonts/releases
function reveal-nf {
  typeset IFS=$'\n'
  typeset -rA name_icons=(
    # You can add new icons here to be used based on name of files.
    ".clang-format" " "
    ".editorconfig" " "
    ".gitignore"    " "
    "LICENSE"       " "
    "Makefile"      " "
  )
  typeset -rA extension_icons=(
    # You can add new icons here to be used based on files' extensions.
    "1"         " "
    "2"         " "
    "3"         " "
    "4"         " "
    "5"         " "
    "6"         " "
    "7"         " "
    "8"         " "
    "bmp"       "󰈟 "
    "c"         " "
    "conf"      " "
    "cpp"       " "
    "cs"        " "
    "css"       " "
    "diff"      "󰦓 "
    "go"        " "
    "gz"        "󰛫 "
    "h"         " "
    "hpp"       " "
    "html"      " "
    "info"      " "
    "iso"       "󱛟 "
    "java"      "󰬷 "
    "jpeg"      "󰈟 "
    "jpg"       "󰈟 "
    "js"        "󰌞 "
    "json"      "󰘦 "
    "lua"       " "
    "md"        " "
    "mk"        " "
    "o"         " "
    "otf"       " "
    "out"       " "
    "php"       " "
    "png"       "󰈟 "
    "py"        "󰌠 "
    "rb"        "󰴭 "
    "rs"        " "
    "scss"      " "
    "sh"        " "
    "tar"       "󰛫 "
    "tex"       " "
    "toml"      "󰬛 "
    "ts"        "󰛦 "
    "ttf"       " "
    "ufo"       " "
    "xcf"       "󰃣 "
    "xml"       "󰗀 "
    "yml"       " "
    "zip"       "󰛫 "
    "zsh"       " "
    "zsh-theme" "󰏘 "
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
