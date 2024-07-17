use strict;
use warnings;

use autodie;

my $file_handle;

open( $file_handle, "<", "cart_types.txt" );

while( my $line = $file_handle->getline() ) {
    my @fields = split( ' ', $line );

#        print join( "|", @fields );

    # $fields[ 0 ] is the cart type
    # $fields[ 1 ] is the description of the cart type

    my $code = shift @fields;

    print "{ " . $code . ", \"" . join( " ", @fields ) . "\" },\n" ;
}
