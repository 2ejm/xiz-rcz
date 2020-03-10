#!/usr/bin/env perl
#
# Copyright (C) 2016 Kurt Kanzenbach <kurt@linutronix.de>
# Time-stamp: <2017-01-16 17:02:31 kurt>
#
# Demo Input Processor Perl script which can be used for testing purposes and
# called e.g. by dataIn and USB/LAN watchdogs.
#
# Command line is: <input_processor> --file <file>
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
my ($file);

sub print_usage_and_die
{
    print STDERR "$0 --file <file>\n";
    exit(-1);
}

sub get_args
{
    GetOptions("file=s" => \$file) ||
        print_usage_and_die();
    print_usage_and_die() unless defined $file;
}

sub get_some_config_value
{
    my ($zix, $ref);

    # connect to LibZIX
    $zix = LibZIX::SocketClient->new() ||
        croak "Failed to create socket client: $!";

    # Sample getConf call: $ref->{body} will contain the xml body of response.
    # The body can be parsed by LibXML. But $ref->{status} should be checked first.
    $ref = $zix->getConf(item => "parameter", id => "serialnumber") or
        croak "Failed to perform LibZIX request";

    $zix->close_socket;

    return "example";
}

sub main
{
    chdir($FindBin::RealBin) || croak "Failed to change to script directory";

    get_args();

    # do something...
    get_some_config_value();
    print "Did something...\n";
}

main();

exit 0;
