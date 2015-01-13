#!/usr/bin/tclsh

# We begin by specifying the shell to run under and initializing some variables.

set VerboseFlag false
set FileToRead "-"
set Usage "Usage: concord \[-v] \[-f<filename>]"

# Remembering that argv is the program arguments array, parse the command line arguments.
# Set FileToRead and give help.

foreach arg $argv {
    switch -glob -- $arg {
        -v      {set VerboseFlag true}
        -f*     {set FileToRead [string range $arg 2 end]}
        -h      {puts stderr $Usage; exit 1}
        default {error "bad argument: $arg\n$Usage"; exit 1}
    }
}

# Set the default input source to the standard input.
# If a file has been specified, open it 'safely'.

set Input stdin

if {$FileToRead != "-"} {
    if [catch {set Input [open $FileToRead r]} res ] {
        puts stderr "$res"
        exit 1
    }
}

# Initialize the word and line counters, then read each line in the input,
# split the line according to punctuation and increment a concordance array.

set NumberOfLines 0
set NumberOfWords 0

while {[gets $Input line] >= 0} {
    incr NumberOfLines
    set words [split $line " \t.,\{\}\(\)\[\]\;\""]
    foreach word $words {
        if {[info exists concord("$word")]} {
            incr concord("$word")
        } else {
            set concord("$word") 1
        }
        incr NumberOfWords
    }
}

# Output a summary, then all the words found, then each word accompanied by its count.

puts stdout [format "File contained %d/%d words/lines\n" \
        $NumberOfWords $NumberOfLines]

puts stdout [array names concord]

foreach word [array names concord] {
    puts "$word: $concord($word)"
}

