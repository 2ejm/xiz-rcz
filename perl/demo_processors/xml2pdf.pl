#!/usr/bin/env perl
#
# Copyright (C) 2016 Kurt Kanzenbach <kurt@linutronix.de>
# Time-stamp: <2017-01-16 17:02:01 kurt>
#
# Demo Post Processor Perl script which can be used for testing purposes and
# called e.g. by dataOut or getMeasurement.
#
# This Demo Script takes the measurement iid and format and creates an empty PDF
# for it.
#

use strict;
use warnings;
use IO::Socket::INET;
use XML::LibXML;
use Carp;
use Getopt::Long;
use FindBin;
use lib "$FindBin::RealBin/..";
use LibZIX::SocketClient;

$| = 1;

# command line arguments specified by specification
my ($iid, $format);

sub print_usage_and_die
{
    print STDERR "$0 --iid <measurement>\n";
    exit(-1);
}

sub get_args
{
    GetOptions("iid=s"    => \$iid,
               "format=s" => \$format) ||
                   print_usage_and_die();
    print_usage_and_die() unless defined $iid;
    print_usage_and_die() unless defined $format;
}

sub get_measurement_dir
{
    my ($zix, $ref);

    # connect to LibZIX
    $zix = LibZIX::SocketClient->new() ||
        croak "Failed to create socket client: $!";

    # Sample getConf call: $ref->{body} will contain the xml body of response.
    # The body can be parsed by LibXML. But $ref->{status} should be checked first.
    $ref = $zix->getConf(item => "directory", id => "measurements") or
        croak "Failed to perform LibZIX request";

    $zix->close_socket;

    return "/usr/local/zix/measurements";
}

sub main
{
    my ($measurement_dir);

    chdir($FindBin::RealBin) || croak "Failed to change to script directory";

    get_args();

    # create empty pdf
    $measurement_dir = get_measurement_dir();
    $measurement_dir .= "/$iid";
    `touch $measurement_dir/$iid.pdf`;
    croak "Failed to create pdf \"$measurement_dir/$iid.pdf\"" if $?;
}

main();

exit 0;
