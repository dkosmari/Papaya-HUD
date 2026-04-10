#!/bin/bash

GREEN=$(tput setaf 10)
YELLOW=$(tput setaf 11)
NONE=$(tput sgr0)


update_submodule()
{
    echo "${GREEN}${1}${NONE}: '${YELLOW}git submodule update --recursive${NONE}'"
    (cd "$1" && git submodule update --recursive) || exit 1
}


update_submodule libmappedmemory
update_submodule libwupsxx

exit 0
