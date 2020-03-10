#!/usr/bin/env perl
#
# Copyright (C) 2016 Kurt Kanzenbach <kurt@linutronix.de>
# Time-stamp: <2017-01-16 17:01:46 kurt>
#
# Demo Allocation Processor Perl script which can be used for testing purposes
# and called by dataSync.
#
# This Demo Script takes the measurement iid and prints dummy patient data to
# stdout.
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
my ($iid);

sub print_usage_and_die
{
    print STDERR "$0 --iid <measurement>\n";
    exit(-1);
}

sub get_args
{
    GetOptions("iid=s" => \$iid) || print_usage_and_die();
    print_usage_and_die() unless defined $iid;
}

sub get_database_conn_infos
{
    my ($zix, $ref);

    # connect to LibZIX
    $zix = LibZIX::SocketClient->new() ||
        croak "Failed to create socket client: $!";

    # Sample getConf call: $ref->{body} will contain the xml body of response.
    # The body can be parsed by LibXML. But $ref->{status} should be checked first.
    $ref = $zix->getConf(item => "parameter", id => "daDatabaseHost") or
        croak "Failed to perform LibZIX request";

    $zix->close_socket;

    return [ "host", "database", "user", "password" ];
}

sub main
{
    my ($measurement_dir, $ref);

    chdir($FindBin::RealBin) || croak "Failed to change to script directory";

    get_args();

    # get database connection infos via getConf
    $ref = get_database_conn_infos();

    # emit dummy patient data
    print << 'EOF';
<patient id="Mustermann, Max 08154711">
  <name>Mustermann</name>
  <surname>Max</surname>
  <gender>male</gender>
  <id>08154711</id>
  <issuer>iCom Server 4343</issuer>
</patient>
EOF
}

main();

exit 0;
