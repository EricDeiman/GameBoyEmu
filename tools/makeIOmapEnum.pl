use strict;
use warnings;

use autodie;

my $file_handle;

open( $file_handle, "<", "iomap.txt" );

while( my $line = $file_handle->getline() ) {
    my @fields = split( / |\t/, $line );

#    print join( "|", @fields );

    # $fields[ 4 ] is the name of the IO channel
    $fields[ 4 ] =~ s/\///;

    # $fields[ 1 ] is the address of the IO channel
    $fields[ 1 ] =~ s/\$//;

    print $fields[ 4 ] . " = 0x" . $fields[ 1 ] . ",\n" ;
}
