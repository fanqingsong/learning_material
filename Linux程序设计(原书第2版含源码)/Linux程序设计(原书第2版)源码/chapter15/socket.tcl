#! /usr/bin/tclsh

set sockid [socket localhost 13]
gets $sockid date
puts "Machine says the date is: $date"
