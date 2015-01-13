#! /usr/bin/perl -w

# Perl translation of chapter 2's shell CD database
# Copyright (C) 1999 Wrox Press.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-#1307  USA

# Now the global variables.

$menu_choice="";
$title_file="title.cdb";
$tracks_file="tracks.cdb";
$temp_file="/tmp/cdb.$$";
$SIG{INT} = sub { tidy_up(); exit; } ;

sub read_in {
    open TITLES, $title_file or die "Couldn't open $title_file : $!\n";
    while (<TITLES>) { chomp; push @titles, $_ };
    close TITLES;

    open TRACKS, $tracks_file or die "Couldn't open $tracks_file : $!\n";
    while (<TRACKS>) { chomp; push @tracks, $_ };
    close TRACKS;
}

sub tidy_up {
    # Die aborts with an error, and $! is the error message from open()
    open TITLES, ">".$title_file or die "Couldn't write to $title_file : $!\n";
    foreach (@titles) { print TITLES "$_\n"; }
    close TITLES;

    open TRACKS, ">".$tracks_file or die "Couldn't open $tracks_file : $!\n";
    foreach (@tracks) { print TRACKS "$_\n"; }
    close TRACKS;
}

# Two little functions for getting keyboard input.

sub get_return {
    print "Press return ";
    <> # Get a line from STDIN, and ignore it.
}

sub get_confirm {
    print "Are you sure? ";
    while (1) {
        $_ = <>; # Get a reply into $_
        return 1 if (/^y(?:es)?$/i); # 1 is true, not 0
        if (/^no?$/i) {
            print "Cancelled!\n";
            return 0;
        }
        print "Please enter yes or no.\n";
    }
}

# Now we display the main menu, and get a choice from the user.
# The <<EOF syntax is called a here-document and prints until it finds the word EOF
# or whatever delimiter string you choose.

sub set_menu_choice {
    print `clear`; # Shelling out to clear screen. Yuck.

    print <<EOF;
    a) Add new CD
    f) Find CD
    c) Count the CDs and tracks in the catalog
EOF

    if ($cdcatnum) {
        print "   l) List tracks on $cdtitle\n";
        print "   r) Remove $cdtitle\n";
        print "   u) Update track information for $cdtitle\n";
    }

    print "   q) Quit\n\n";
    print "Please enter choice then press return\n";
    chomp($menu_choice=<>);
    return
}

# Then the one-liners as before to add new records to the arrays
# and the subroutine to add track information.

sub insert_title {
    push @titles, (join "," , @_);
}

sub insert_track {
    push @tracks, (join "," , @_);
}

sub add_record_tracks {
    print "Enter track information for this CD\n";
    print "When no more tracks enter q\n";
    $cdtrack=1;
    $cdttitle="";
    while ($cdttitle ne "q") {
        print "Track $cdtrack, track title? ";
        chomp($cdttitle=<>);
        if ($cdttitle =~ /,/) {
            print "Sorry, no commas allowed.\n";
            redo;
        }
        if ($cdttitle and $cdttitle ne "q") {
            insert_track($cdcatnum,$cdtrack,$cdttitle);
            $cdtrack++;
        }
    }
}

# Now we implement the add_records function to add the record of a new CD to the database.

sub add_records {
    print "Enter catalog name ";
    chomp($cdcatnum=<>);
    $cdcatnum=~ s/,.*//; # Drop everything after a comma.

    print "Enter title ";
    chomp($cdtitle=<>);
    $cdtitle =~ s/,.*//;

    print "Enter type ";
    chomp($cdtype=<>);
    $cdtype =~ s/,.*//;

    print "Enter artist/composer ";
    chomp($cdac=<>);
    $cdac =~ s/,.*//;

    print "About to add a new entry\n";
    print "$cdcatnum $cdtitle $cdtype $cdac\n";

    if (get_confirm()) {
        insert_title($cdcatnum,$cdtitle,$cdtype,$cdac);
        add_record_tracks();
    } else {
        remove_records();
    }
}

# Since we've got an array of lines, finding the CD is very simple.
# We could iterate through the array and pick out the matches.
# However, it's easier to use Perl's grep function which was intended for this very purpose.

sub find_cd {
    # $asklist is true if the first member of @_
    # (That is, the first parameter) is not "n"
    $asklist = ($_[0] ne "n");

    $cdcatnum="";
    print "Enter a string to search for in the CD titles ";
    chomp($searchstr=<>);
    return 0 unless $searchstr;

    # The \Q and \E metacharacters stop other metacharacters
    # from working, so question marks, asterisks and so on
    # in titles aren't dangerous.
    
    @matches = grep /\Q$searchstr\E/, @titles;
    if (scalar @matches == 0) {
        print "Sorry, nothing found.\n";
        get_return();
        return 0;
    } elsif   (scalar @matches != 1 ) {
        print "Sorry, not unique.\n";
        print "Found the following:\n";
        foreach (@matches) {
            print "$_\n";
        }
        get_return();
        return 0;
    }

    ($cdcatnum,$cdtitle,$cdtype,$cdac) = split "," , $matches[0];
    unless ($cdcatnum) {
        print "Sorry, could not extract catalog field\n";
        get_return();
        return 0;
    }
    print "\nCatalog number: $cdcatnum\n";
    print "Title: $cdtitle\n";
    print "Type: $cdtype\n";
    print "Artist/Composer: $cdac\n\n";
    get_return();
    if ($asklist) {
        print "View tracks for this CD? ";
        $_ = <>;
        if (/^y(?:es)?$/i) {
            print "\n";
            list_tracks();
            print "\n";
        }
    }
    return 1;
}

# update_cd is nice and easy to implement, apart from the bit where we delete
# the old tracks from the array. We'll do this using another grep,
# but this time, we can negate the regular expression using !/regexp/. 

sub update_cd {
    unless ($cdcatnum) {
        print "You must select a CD first\n";
        find_cd("n");
    }
    if ($cdcatnum) {
        print "Current tracks are :-\n";
        list_tracks();
        print "\nThis will re-enter the tracks for $cdtitle\n";
        if (get_confirm()) {
            @tracks = grep !/^$cdcatnum,/, @tracks;
            add_record_tracks();
        }
    }
}

# Since it's all stored in arrays, counting the contents of the database is trivial. 

sub count_cds {
    print "Found ".(scalar @titles)." CDs, ";
    print "with a total of ".(scalar @tracks)." tracks.\n";
    get_return();
}

# We've seen how to use grep with a negated regexp to remove entries from an array.
# Let's do this again: 

sub remove_records {
    unless ($cdcatnum) {
        print "You must select a CD first\n";
        find_cd("n");
    }

    if ($cdcatnum) {
        print "You are about to delete $cdtitle\n";
        if (get_confirm()) {
            @titles = grep !/^$cdcatnum,/, @titles;
            @tracks = grep !/^$cdcatnum,/, @tracks;
            @cdcatnum="";
            print "Entry removed";
        }
        get_return();
    }
}

# list_tracks requires a pager, so we need to write out a temporary file and shell out.

sub list_tracks {
    unless ($cdcatnum) {
        print "No CD selected yet.\n";
        return
    }
    open(TEMP, ">$temp_file")
         or die "Can't write to $temp_file: $!\n";
    @temp = grep /^$cdcatnum,/ , @tracks;
    if (scalar @temp == 0) {
        print "No tracks found for $cdtitle\n";
    } else {
        print TEMP "\n$cdtitle :-\n\n";
        foreach (@temp) {
            s/^.*?,//; # Remove the first field
            print TEMP $_."\n";
        }
        close TEMP;
        system("more $temp_file");
        unlink($temp_file); # Delete it.
    }
    get_return();
}

# Now the main routine; we must remember to write out the arrays before exiting.
# We also make sure the files exist before reading from them, by creating them.
# Of course, we needn't have done it this way.
# The alternative is not to complain if the files do not exist, the arrays would be empty
# and the files would be created when we leave. 

# File tests work like shell
system("touch $title_file") unless ( -f $title_file );
system("touch $tracks_file") unless ( -f $tracks_file );

read_in();

system("clear");
print "\n\nMini CD manager\n";
sleep(3);

while (1) {
    set_menu_choice();
    if ($menu_choice =~ /a/i) { add_records(); }
    elsif ($menu_choice =~ /r/i) { remove_records(); }
    elsif ($menu_choice =~ /f/i) { find_cd("y"); }
    elsif ($menu_choice =~ /u/i) { update_cd(); }
    elsif ($menu_choice =~ /c/i) { count_cds(); }
    elsif ($menu_choice =~ /l/i) { list_tracks(); }
    elsif ($menu_choice =~ /b/i) {
        print "\n";
        foreach (@titles) {
            print "$_\n";
        }
        print "\n";
        get_return();
    }
    elsif ($menu_choice =~ /q/i) { last; }
    else { print "Sorry, choice not recognized.\n"; }
}

tidy_up();
exit;
