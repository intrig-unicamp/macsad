#!/bin/bash -e

make ${@} 2>&1 | perl -wln -M'Term::ANSIColor' -e '
m/^([^ \t]*:|)([ \t]*)(fatal error|Error|multiple definition of|undefined reference to)(.*)/i and print "\e[1;36m", "$1\n$2", "\e[1;91m", "$3", "$4", "\e[0m"
or
m/^([^ \t]*:|)([ \t]*)(Warning|note:)(.*)/i and print "\e[1;36m", "$1\n$2", "\e[1;93m", "$3", "$4", "\e[0m"
or
m/^([^ \t]*:|)([ \t]*)(Linking|\.a\b)(.*)/ and print "\e[1;36m", "$1\n$2", "\e[1;36m", "$3", "$4", "\e[0m"
or
m/^([^ \t]*:|)([ \t]*)(Building|gcc|g++|\bCC\b|\bLD\b|\bINSTALL-APP\b|\bINSTALL-MAP\b|\bcc\b)(.*)/ and print "\e[1;36m", "$1$2", "\e[1;32m", "$3", "$4", "\e[0m"
or
print; '

