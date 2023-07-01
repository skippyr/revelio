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
    # You can add new icons here to be used based on files' names.
    ".clang-format" " :3"
    ".editorconfig" " :8"
    ".gitignore"    " :1"
    "LICENSE"       " :3"
    "Makefile"      " :3"
  )
  typeset -rA extension_icons=(
    # You can add new icons here to be used based on files' extensions.
    "1"         " :4"
    "2"         " :4"
    "3"         " :4"
    "4"         " :4"
    "5"         " :4"
    "6"         " :4"
    "7"         " :4"
    "8"         " :4"
    "avi"       "󰈫 :6"
    "bmp"       "󰈟 :5"
    "c"         " :4"
    "conf"      " :8"
    "cpp"       " :5"
    "cs"        " :4"
    "css"       " :4"
    "diff"      "󰦓 :1"
    "doc"       " :4"
    "docx"      " :4"
    "go"        " :4"
    "gz"        "󰛫 :8"
    "h"         " :4"
    "hpp"       " :5"
    "html"      " :1"
    "info"      " :4"
    "iso"       "󱛟 :8"
    "java"      "󰬷 :8"
    "jpeg"      "󰈟 :5"
    "jpg"       "󰈟 :5"
    "js"        "󰌞 :3"
    "json"      "󰘦 :3"
    "lua"       " :4"
    "md"        " :4"
    "mk"        " :8"
    "mov"       "󰈫 :6"
    "mp3"       "󰈣 :3"
    "mp4"       "󰈫 :6"
    "o"         " :2"
    "otf"       " :4"
    "out"       " :2"
    "pdf"       " :1"
    "php"       " :5"
    "png"       "󰈟 :5"
    "ppt"       "󱎐 :1"
    "pptx"      "󱎐 :1"
    "py"        "󰌠 :4"
    "rb"        "󰴭 :1"
    "rs"        " :1"
    "scss"      " :5"
    "sh"        " :2"
    "tar"       "󰛫 :8"
    "tex"       " :4"
    "toml"      "󰬛 :1"
    "ts"        "󰛦 :4"
    "ttf"       " :4"
    "ufo"       " :4"
    "vbox"      " :8"
    "vbox-prev" " :8"
    "vdi"       "󱛟 :8"
    "wav"       "󰈣 :3"
    "xcf"       "󰃣 :8"
    "xls"       "󱎏 :2"
    "xlsx"      "󱎏 :2"
    "xml"       "󰗀 :1"
    "yml"       " :1"
    "zip"       "󰛫 :8"
    "zsh"       " :2"
    "zsh-theme" "󰏘 :8"
  )
  typeset -r directory_icon=" :3"
  typeset -r file_icon=" :8"
  typeset paths=($@)
  [[ ${#paths[@]} -eq 0 ]] &&
    paths+=(".")
  typeset output=""
  for path_ in ${paths[@]}; do
    if [[ ! -d "${path_}" ]]; then
      output+="$0: \"${path_}\" is not a directory.\n"
      continue
    fi
    [[ -n "${output}" ]] &&
      output+="\n"
    output+="${path_}:\n"
    for entry in $(reveal "${path_}" | sort); do
      typeset name="${entry##*/}"
      output+="  "
      if [[ -d "${entry}" ]]; then
        output+="\x1b[3${directory_icon:3:3}m${directory_icon:0:2}\x1b[0m"
      else
        typeset extension="${entry##*.}"
        typeset name_icon="${name_icons[${name}]:0:2}"
        if [[ "${name_icon}" != "" ]]; then
          output+="\x1b[3${name_icons[${name}]:3:3}m${name_icon}\x1b[0m"
        else
          typeset extension_icon="${extension_icons[${extension}]:0:2}"
          if [[ "${extension_icon}" != "" ]]; then
            output+="\x1b[3${extension_icons[${extension}]:3:3}m${extension_icon}\x1b[0m"
          else
            output+="\x1b[3${file_icon:3:3}m${file_icon:0:2}\x1b[0m"
          fi
        fi
      fi
      output+="  ${name}\n"
    done
  done
  echo -e "${output}"
}
