#!/usr/bin/perl -w

%phonebook = ( "Bob" => "247305", "Phil" => "205832", "Sara" => "226010" );

# We created our hash just like a list, but used the => operator
# (which is equivalent to a comma in most cases) to separate our key-value pairs.

print "Sara's phone number is ", $phonebook{"Sara"}, "\n";
print "Bob's phone number is ", $phonebook{Bob}, "\n";

$phonebook{Bob} = "293028";
print "Bob's new number is ", $phonebook{Bob}, "\n";
