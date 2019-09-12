# vi:syntax=sh
#
# install-cli
#
# See install.example for usage.
#

#
# start installation lib
#

# extended glob syntax needed by require()
shopt -s extglob

#
# messaging

icli_BASECONTEXT="install"
icli_CONTEXT=$icli_BASECONTEXT

icli::set_context() {
  icli_CONTEXT=$1
}

icli::unset_context() {
  icli_CONTEXT=$icli_BASECONTEXT
}

readonly T_0=$(tput sgr0)

readonly T_DIM=$(tput dim)

readonly T_UNDER=$(tput smul)
readonly T_NO_UNDER=$(tput rmul)

readonly T_FCYAN=$(tput setaf 6)
readonly T_FGREEN=$(tput setaf 2)
readonly T_FMAGENTA=$(tput setaf 5)
readonly T_FRED=$(tput setaf 1)
readonly T_FYELLOW=$(tput setaf 3)

icli::message() {
  echo -e "$T_DIM($icli_CONTEXT)$T_0" $@ $T_0
}

icli::context_message() {
  icli::set_context "$1"
  icli::message "$2"
  icli::unset_context
}

#
# misc helpers

icli::defn_terse() {
  # NOTE: We were doing the following, in order to trim the function
  # declaration's enclosing brackets:
  #
  #   declare -f $1 | tail -n +3 | head -n -1 | ...
  #
  # However, not all implementations of `head` support negative line
  # numbers.
  #
  # And, rather than any of the crazy sed/awk/etc. that would be
  # required to reproduce this functionality, we'll just leave the
  # enclosing brackets now!
  #
  # declare -f $1 | tail -n +3 | head -n -1 | sed 's/^[[:blank:]]*//' | paste -s -d ' '

  # NOTE: POSIX paste requires specification of file ("-" for stdin)
  declare -f $1 | tail -n +2 | sed 's/^[[:blank:]]*//' | sed 's/[[:blank:]]*$//'| paste -s -d ' ' -
}

icli::join_by() {
  local IFS="$1"
  shift
  echo "$*"
}

icli::vercomp() {
  # compare two version specs
  #
  # returns:
  #
  #   0: identical
  #   1: first is greater
  #   2: second is greater
  #
  if [[ $1 == $2 ]]; then
    return 0
  fi

  local IFS=.
  local i ver1=($1) ver2=($2)

  # fill empty fields in ver1 with zeros
  for ((i=${#ver1[@]}; i<${#ver2[@]}; i++)); do
    ver1[i]=0
  done

  for ((i=0; i<${#ver1[@]}; i++)); do
    if [[ -z ${ver2[i]} ]]; then
      # fill empty fields in ver2 with zeros
      ver2[i]=0
    fi
    if ((10#${ver1[i]} > 10#${ver2[i]})); then
      return 1
    fi
    if ((10#${ver1[i]} < 10#${ver2[i]})); then
      return 2
    fi
  done

  return 0
}

#
# require()

readonly icli_REQUIRE_ARGS=(name checker installer fail-prefix fail-message fail-with)
readonly icli_REQUIRE_OPTIONS=(${icli_REQUIRE_ARGS[@]:2})
readonly icli_OPTION_PATTERN="@($(icli::join_by "|" ${icli_REQUIRE_OPTIONS[@]}))"

icli_STARTED=false

require() {
  # if require() is ever called,
  # print the "begin" message;
  # but, only once:
  $icli_STARTED || {
    icli_STARTED=true
    icli::message "${T_FCYAN}begin"
  }

  echo

  # parse positional arguments and options
  # (Bash doesn't support --long-options, and Darwin doesn't supply
  # GNU getopt by default)
  local -i index=0
  local pair key value

  while [ "${1+defn}" ]
  do
    case "$1" in
      --$icli_OPTION_PATTERN=*)
        pair=${1:2}  # strip dashes
        key=${pair%=*}  # split key and value
        value=${pair#*=}
        ;;
      -*)
        icli::message "${T_FRED}error: require has no such option '$1'"
        return 1
        ;;
      *)
        [ $index -eq ${#icli_REQUIRE_ARGS[@]} ] && {
          icli::message "${T_FRED}error: require received too many arguments"
          return 1
        }
        key=${icli_REQUIRE_ARGS[$index]}
        value="$1"
        ((index++))
        ;;
    esac
    declare -r ${key//-/_}="$value" 2> /dev/null || {
      icli::message "${T_FRED}error: argument '$key' specified multiple times"
      return 1
    }
    shift
  done

  if [ -z "$name" ] || [ -z "$checker" ]
  then
    icli::message "${T_FRED}error: require received too few arguments"
    return 1
  fi

  local result

  # check (and install)
  icli::set_context $name

  if $checker
  then
    [ -z "$installer" ] && local success_word="check passed" || local success_word="installed"
    icli::message "${T_FGREEN}${success_word} ✓"
    result=0
  elif [ -z "$installer" ]
  then
    if [ -n "$fail_message" ]; then
      icli::message "$T_FRED$fail_message"
    else
      icli::message ${fail_prefix:+$T_FRED$fail_prefix –} "${T_FRED}check failed ✗"
    fi
    result=1
  else
    if [ -n "$fail_message" ]; then
      icli::message "$T_FYELLOW$fail_message"
    else
      icli::message ${fail_prefix:+$T_FYELLOW$fail_prefix –} "${T_FYELLOW}install?"
    fi
    echo -n $T_DIM  # hack the select menu
    select _yn in "${T_0}${T_FYELLOW}${T_UNDER}yes, install$T_0 $T_DIM$(icli::defn_terse $installer)" \
                  "${T_0}${T_FMAGENTA}no, ignore$T_0$T_DIM"
    do
      case $REPLY in
        1)
          echo -n $T_0  # reset from hack (in case of installer standard output)
          $installer
          local install_success=$?
          if [ "$checker" != "icli::always_install" ]
          then
            $checker
            install_success=$?
          fi
          if [ $install_success -eq 0 ]
          then
            icli::message "${T_FGREEN}installed ✓"
            result=0
          else
            icli::message "${T_FRED}installation failed ✗"
            result=1
          fi
          break;;
        2)
          icli::message "${T_FYELLOW}will not install ✗"
          result=1
          break;;
      esac
    done
  fi

  if [ -n "$fail_with" ] && [ $result = 1 ]; then
    icli::message "${T_FYELLOW}$fail_with"
    exit 1
  fi

  icli::unset_context
}

#
# helpers intended for user scripts

icli::always_install() {
  return 1
}

icli::check_command() {
  command -v "$1" > /dev/null
}

icli::check_envvars() {
  local var
  for var in "$@"
  do
    [ -z "${!var}" ] && return 1
  done
  return 0
}

#
# exit handling

# if require() was ever called,
# print the "done" message, on exit

icli_FINISHED=false

icli::finish() {
  if [[ $icli_STARTED = true && $icli_FINISHED = false ]]
  then
    echo
    icli::unset_context
    icli::message "${T_FCYAN}${1}"
    icli_FINISHED=true
  fi
}

icli::finish_int() {
  icli::finish interrupted
  exit 130
}

icli::finish_exit() {
  icli::finish done
}

trap icli::finish_int INT
trap icli::finish_exit EXIT

#
# end installation lib
#
