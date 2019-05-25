#! /usr/bin/perl -w

# Perl translation of chapter 2's shell CD database
# Copyright (C) 1999 Wrox Press.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-#1307 USA


# We'll place the whole of the program here, and flesh out the functions later.

use MLDBM qw(AnyDBM_File);
my $record;
tie(%tmp, "MLDBM", "cddb.db")
or die "Couldn't tie DB.\n"; # Scary complex hash contains the whole DB.
%database = %tmp; # Overcome a limitation in MLDBM. *sigh*

# Tidy up nicely
$SIG{INT} = sub { %tmp = %database; untie %tmp } ;

system("clear");
print "\n\nCD Database Manager\n\n";

while (1) {
    my $menu_choice = main_menu($record);
    if ($menu_choice eq "a") { $record = add_cd(); }
    elsif ($menu_choice eq "r") { remove_cd($record); undef $record; }
    elsif ($menu_choice eq "f") { $record = find_cd("y"); }
    elsif ($menu_choice eq "u") { update_cd($record); }
    elsif ($menu_choice eq "c") { count_cds(); }
    elsif ($menu_choice eq "l") { list_tracks($record); }
    elsif ($menu_choice =~ /q/i) { last; }
    else {
        print "Can't get here.\n";
    }
}

%tmp=%database;
untie %tmp;

# Now we display the main menu and validate the choice.

sub main_menu {
    my $record = shift;
    my $choice;
    my $title = $database{$record}->{title} if $record;
    print <<EOF;
    
    Options :
        a) Add new CD
        f) Find CD
        c) Count CDs and tracks in the catalogue
EOF
    
    if ($record) {
        print "   l) List tracks on $title\n";
        print "   r) Remove $title\n";
        print "   u) Update entry for $title\n";
    }
    print "   q) Quit\n";
    print "Your choice: ";
    while (1) {
        $choice=lc(<>);
        substr($choice,1)="";
        # Now, we see if the choice is contained in the string of
        # acceptable options, (Which includes l, r and u if we've
        # selecetd a record.) by using it as a regexp. Looks weird?
        return $choice if ("afcq".($record?"lru":"") =~ /$choice/);
        # If not, that's invalid
        print "Invalid choice.\nTry again: ";
    }
}

# Let's tackle adding records to the database next. The database is actually
# quite a complicated hash; the keys are the catalog numbers, and the values
# are each themselves hashes. These hashes have keys "title", "type", "artist",
# and "tracks". That's why we used the funky-looking $database{$record}->{title}
# above - $database{$record} is a hash (it's actually a reference to a hash;
# C programmers can think of them as pointers. For more about reference, look
# at the perlref documentation) and the ->{title} syntax looks inside the hash
# reference and gets the value of the "title" key. The value of "tracks" is,
# of course, a reference to an array of tracks. Arrays inside hashes inside hashes.
# It takes a little getting used to. 

sub add_cd {
    while(1) {
        print "Enter catalog number: ";
        chomp($record=<>);
        if (exists $database{$record}) {
            print "Already exists. ";
            print "Please enter a different number.\n";
        } else {
            last;
        }
    }
    
    print "Enter title: ";
    chomp($title=<>);
    
    print "Enter type: ";
    chomp($type=<>);
    print "Enter artist/composer: ";
    chomp($artist=<>);

    $database{$record}= {
        "title" => $title,
        "type" => $type,
        "artist" => $artist
    };

    add_tracks($record);
    return $record; # Tell the main menu the new record number.
}

# Now the subroutine to add the tracks; this is where we bring out the array reference. 

sub add_tracks {
    my $record = shift;
    print "Enter track information for this CD\n";
    print "Enter a blank line to finish.\n\n";
    my $counter=0; my @tracks;
    while (1) {
        print ++$counter.": ";
        chomp($track=<>);
        if ($track) {
            # @{...} means "interpret as an array"
            push @{$database{$record}->{tracks}}, $track;
        } else {
            last;
        }
    }
}


sub find_cd {
    $view = ($_[0] eq "y");

    print "Enter a string to search for: ";
    chomp($search=<>);

    # For each key, (record) add the key to the @found array if the
    # title field of that record contains the search string.
    @matches = grep {$database{$_}->{title} =~/\Q$search\E/ }
    keys %database;

    if (scalar @matches == 0) {
        print "Sorry, nothing found.\n";
        return;
    } elsif (scalar @matches != 1 ) {
        print "Sorry, not unique.\n";
        print "Found the following:\n";
        foreach (@matches) {
            print $database{$_}->{title}."\n";
        }
        return;
    }
    $record=$matches[0];
    print "\n\nCatalog number: ".$record."\n";
    print "Title: ".$database{$record}->{title}."\n";
    print "Type: ".$database{$record}->{type}."\n";
    print "Artist/Composer: ".$database{$record}->{artist}."\n\n";

    if ($view) {
        print "Do you want to view tracks? ";
        $_ = <>;
        if (/^y(?:es)?$/i) {
            print "\n";
            list_tracks($record);
            print "\n";
        }
    }

    return $record;
}

# Once we've got this far, listing the tracks isn't difficult! 

sub list_tracks {
    my $record = shift;
    foreach (@{$database{$record}->{tracks}}) {
        print $_."\n";
    }
}

# Updating a CD just means removing the old tracks and adding a new set.

sub update_cd {
    my $record = shift;
    print "Current tracks are: \n";
    list_tracks($record);
    print "\nDo you want to reenter them?\n";
    if (($_ = <>) =~ /^y(?:es)?$/i) {
        # Remove the old entry from the hash
        delete $database{$record}->{tracks};
        add_tracks($record);
    } else {
        print "OK, canceling.\n";
    }
}

# Similarly, removing a CD just means deleting its hash entry.

sub remove_cd {
    my $record = shift;
    print "\nDo you want to delete this CD?\n";
    if (($_ = <>) =~ /^y(?:es)?$/i) {
        delete $database{$record};
    } else {
        print "OK, cancelling.\n";
    }
}

# Finally, counting the CDs is easy - it's just the number of keys in the hash.
# Counting the tracks, however, is a little more tricky; we evaluate the tracks
# array in scalar context for each of the keys in the database, and add the
# values together. (You could do this with map() but that would be less clear.)

sub count_cds {
    my $totaltracks=0;
    print "Found ".(scalar keys %database)." CDs and ";
    foreach (keys %database) {
        $totaltracks+= scalar @{$database{$_}->{tracks}};
    }
    print $totaltracks." tracks.\n";
}
