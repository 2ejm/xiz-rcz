#!/usr/bin/env perl
#
# Copyright (C) 2016 Kurt Kanzenbach <kurt@linutronix.de>
# Time-stamp: <2017-01-16 17:01:54 kurt>
#
# Demo Serial Handler Perl script which can be used for testing purposes and
# called by dataOut.
#
# This Demo Script takes the parameters specified by specification v2 and return 0.
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
my ($iid, $type, $format, $protocol, $scheme);

# internal stuff
my @allowed_types = (q{measurement}, q{conf}, q{log}, q{monitor});

sub print_usage_and_die
{
    print STDERR "$0 --iid <measurement> --type <measurement|conf|log|monitor> --format <format> --protocol <protocol> --scheme <scheme>\n";
    exit(-1);
}

sub get_args
{
    GetOptions("iid=s"      => \$iid,
               "type=s"     => \$type,
               "format=s"   => \$format,
               "protocol=s" => \$protocol,
               "scheme=s"   => \$scheme) || print_usage_and_die();

    print_usage_and_die() unless defined $iid;
    print_usage_and_die() unless defined $type;
    print_usage_and_die() unless defined $format;
    print_usage_and_die() unless defined $protocol;
    print_usage_and_die() unless defined $scheme;
    print_usage_and_die() unless scalar (grep { $type eq $_ } @allowed_types);
}

sub get_serial_parameters
{
    my ($zix, $ref);

    # connect to LibZIX
    $zix = LibZIX::SocketClient->new() ||
        croak "Failed to create socket client: $!";

    # Sample getConf call: $ref->{body} will contain the xml body of response.
    # The body can be parsed by LibXML. But $ref->{status} should be checked first.
    $ref = $zix->getConf(item => "parameter", resource => "serial") or
        croak "Failed to perform LibZIX request";

    $zix->close_socket;

    return [ "9600" ];
}

sub main
{
    my ($ref);

    chdir($FindBin::RealBin) || croak "Failed to change to script directory";

    get_args();

    $ref = get_serial_parameters();

    # here follows the serial writing part
}

main();

exit 0;
