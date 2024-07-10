# Process the file gameboy_opcodes.html to generate
# the opcode table for use in a C/C++ program

# Orginal opcode file is from:
# https://www.pastraiser.com/cpu/gameboy/gameboy_opcodes.html

# This script is *very* dependent on the format of the opcodes file,
# so I use a local version of the file in case the online one changes

# to run this script:
#   perl makeCPUtable.pl > ../src/CPU/_insr_details.hh

# a helpful script to generate the prototypes of the methods that need to be implemented:
#   sed "s/\"[^\"]*\"//" ../src/CPU/_insr_details.hh | cut -d ',' -f 3 | sort | uniq | sed -E "s/&CPU::(.*)/void \1\( const InstDetails\&, u8, u8 \) { throw std::runtime_error( \"not implemented\" ); }/" > neededFns.txt 

use strict;
use warnings;

use autodie;

my @opcode_info = ();  # array of hashes

sub process_line {
    my $param = shift;
    my @ops = split( /<td/, $param );
    my %rtn = ();

    foreach my $opcode_data ( @ops ) {
        my $op_data = {};

        # filter out html that are not opcode fields
        if( $opcode_data =~ /\s*<tr/ ) {
            next;
        }

        # if a line has a <b>&nbsp; it is a header, not opcode
        if( $opcode_data =~ /<b>&nbsp;/ ) {
            next;
        }

        # trim the field
        $opcode_data =~ /[^>]*>(.*?)<\/td>(<\/tr>)?/;
        $opcode_data = $1;

        if( $opcode_data eq "&nbsp;" ) {
            # push( @op_data, "illegal opcode" );
            $op_data->{ "ins" } = "illegal opcode";
        }
        else {
            # break the opcode data into components
            my @opcode_fields = split( /<br>/, $opcode_data);

            # push( @op_data, $opcode_fields[ 0 ] );
            $op_data->{ "ins" } = $opcode_fields[ 0 ];

            my @counts = split( /\&nbsp\;\&nbsp\;/, $opcode_fields[ 1 ] );
            $op_data->{ "bytes" } = $counts[ 0 ];
            $op_data->{ "cycles" } = $counts[ 1 ];

            # push( @op_data, $opcode_fields[ 2 ] );
            $op_data->{ "flags" } = $opcode_fields[ 2 ];
        }

        push( @opcode_info, $op_data );
    }

    return %rtn;
}

# addressing modes
#   Tells the instruction how to interpret the operands
# am_ins        the instruction bits have all the information on destination and source
#                 this will be the address mode for 1 byte instructions
# am_d8         the insruction encodes the destination register and the operand is 8-bit immediate data (2 bytes)
# am_r8         the instruction encodes the destination register and the operand is 8-bit register (2 bytes)
# am_a16        the operand is a 16-bit address (2 bytes)
# am_ia8        the operand is an indirect 8-bit address (2 bytes)
# am_ia16       the operand is an indirect 16-bit address (3 bytes)

sub process_rand {
    my $param = shift;
    my $indirect = 0;
    my $rda = "";


    # is operand direct or indirect
    if( $param =~ /\(/ ) {
        $indirect = 1;
    }

    # is operand register, data, or address
    if( $param =~ /a8/ ) {
       $rda = "a8";
    }
    elsif( $param =~ /a16/ ) {
        $rda = "a16";
    }
    elsif( $param =~ /d8/ ) {
        $rda = "d8";
    }
    elsif( $param =~ /d16/ ) {
        $rda = "d16";
    }
    elsif( $param =~ /r8/ ) {
        $rda = "r8";
    }
    elsif( $param =~ /^0$/ ) {
        $rda = "ins";
    }

    if( $indirect == 1 && $rda eq "" ) {
        $indirect = 0;
    }

    return sprintf( "%s%s",  ($indirect == 1 ? "i" : ""), $rda );
}

sub process_opcode {
    my $parm = shift;
    my $byte_cnt = shift;
    my @op_details = split( /[ ,]/, $parm );
    my $op_name = $op_details[ 0 ];
    my $rand1 = "";
    my $rand1len = 0;
    my $rand2 = "";
    my $rand2len = 0;
    my $am = "am";

    if( $byte_cnt > 1 ) {
        if ( ( scalar @op_details ) >= 2 ) {
            $rand1 = process_rand( $op_details[ 1 ] );
            $rand1len = length( $rand1 );
        }

        if ( ( scalar @op_details ) == 3 ) {
            $rand2 = process_rand( $op_details[ 2 ] );
            $rand2len = length( $rand2 );
        }
    }

    if( $byte_cnt == 1 || ( $rand1len == 0 && $rand2len == 0 ) ) {
        $am = $am . "_ins";
    }
    else {
        if( $rand1len > 0 ) {
            $am = $am . "_" . $rand1;
        }

        if( $rand2len > 0 ) {
            $am = $am . "_" . $rand2;
        }
    }

    return sprintf( "&CPU::%s, %s", $op_name, $am );

}

sub dump_opcode_code {
    my $op_number = 0;
    my $byte_count_adj = 0;

    foreach my $op ( @opcode_info ) {

        # each entry in the hash has at least an "ins" field
        my $count = scalar keys %$op;

        if( $count == 1 ) {
            # illegal instruction
            #           opcode  templ fn     byt  cy1 cy2  Z     N     H     C
            printf( "{ 0x%03x, %-14s, %-20s, %-s, %2s, %2s, '%s', '%s', '%s', '%s' },\n",
                    $op_number,
                    sprintf( "\"%s\"", "illegal" ),
                    "&CPU::ILL, am_ins",
                    1,
                    1,
                    0,
                    "-",
                    "-",
                    "-",
                    "-"
                );
        } else {
            # legal instruction
            my $bytes = $op->{ "bytes" } + $byte_count_adj;
            my $cy1 = 0;
            my $cy2 = 0;

            if( $op->{ "cycles" } =~ /\// ) {
                ( $cy1, $cy2 ) = split( /\//, $op->{ "cycles" } );
            }
            else {
                $cy1 = $op->{ "cycles" };
            }

            my ( $Z, $N, $H, $C ) = split( / /, $op->{ "flags" } );

            #           opcode  templ fn     byt  cylo cyhi  Z     N     H     C
            printf( "{ 0x%03x, %-14s, %-20s, %-s, %2s, %2s, '%s', '%s', '%s', '%s' },\n",
                    $op_number,
                    sprintf( "\"%s\"", $op->{ "ins" } ),
                    process_opcode( $op->{ "ins" }, $bytes ),
                    $bytes,
                    $cy1,
                    $cy2,
                    $Z,
                    $N,
                    $H,
                    $C
                );
        }

        $op_number++;

        if( $op_number > 255 ) {
            $byte_count_adj = -1;
        }
    }
}


my $file_handle;

open( $file_handle, "<", "./gameboy_opcodes.html" );

while( my $line = $file_handle->getline() ) {
    if( $line =~ /^<tr/ ) {
        process_line( $line );
    }
}

if( ( scalar @opcode_info ) != 512 ) {
    my $table_len = scalar @opcode_info;

    print "unexpected number of op codes, $table_len\t\texpected 512\n";
}

dump_opcode_code();
