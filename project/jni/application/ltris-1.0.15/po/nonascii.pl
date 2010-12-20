#!/usr/bin/perl -w
use Text::Unaccent;
open(my $in,  "<",  $ARGV[0])  or die "Can't open $ARGV[0]: $!";

while (<$in>) {     # assigns each line in turn to $_
	print(unac_string('UTF-8', $_));
}
