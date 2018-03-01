# vi:syntax=sh
#
# install-cli
#
# See install.example for usage.
#

#
# start installation lib
#

BASECONTEXT=install
CONTEXT=$BASECONTEXT

set_context() {
  CONTEXT=$1
}

unset_context() {
  CONTEXT=$BASECONTEXT
}

T_0=$(tput sgr0)

T_DIM=$(tput dim)
T_UNDER=$(tput smul)

T_FCYAN=$(tput setaf 6)
T_FGREEN=$(tput setaf 2)
T_FMAGENTA=$(tput setaf 5)
T_FYELLOW=$(tput setaf 3)

message() {
  echo -e "$T_DIM($CONTEXT)$T_0" $@ $T_0
}

defn_terse() {
  declare -f $1 | tail -n +3 | head -n -1 | sed 's/^[[:blank:]]*//' | paste -s -d '; '
}

STARTED=false

require() {
  $STARTED || {
    STARTED=true
    message ${T_FCYAN}begin
  }

  set_context $1
  echo

  if $2
  then
    message "${T_FGREEN}installed ✓"
  else
    message ${4:+$T_FYELLOW$4} "${T_FYELLOW}install?"
    echo -n $T_DIM  # hack the select menu
    select yn in "${T_0}${T_FYELLOW}${T_UNDER}yes, install$T_0 $T_DIM{$(defn_terse $3)}" "${T_0}${T_FMAGENTA}no, ignore$T_0$T_DIM"
    do
      case $REPLY in
        1)
          $3
          message "${T_FGREEN}installed ✓"
          break;;
        2)
          message "${T_FYELLOW}will not install ✗"
          break;;
      esac
    done
  fi

  unset_context
}

always_install() {
  return 1
}

finish() {
  $STARTED && {
    echo
    unset_context
    message ${T_FCYAN}done
  }
}

trap finish EXIT

#
# end installation lib
#
