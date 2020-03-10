#!/usr/bin/env perl
#
# Copyright (C) 2018 Kurt Kanzenbach <kurt@linutronix.de>
# Time-stamp: <2018-08-28 12:05:57 kurt>
#

use strict;
use warnings;
use FindBin;
use lib "$FindBin::RealBin/";
use LibZIX::SocketClient;
use Carp;

my ($host, $port, $zix, $ref, $function) = ("localhost", 17001);

$host = "localhost";
$port = 17001;

if ($ARGV[0]) {
    $function = $ARGV[0];
} else {
    $function .= $_ while (<>);
}

die unless $function;

$zix = LibZIX::SocketClient->new(host => $host, port => $port) or die;

print "-" x 80 . "\n";
print "Sending: $function\n";

$ref = $zix->request($function) or die;
$zix->close_socket();

print "Status: "  . $ref->{status}  . "\n";
print "Message: " . $ref->{message} . "\n" if $ref->{message};
print "Body: "    . $ref->{body}    . "\n" if $ref->{body};
print "-" x 80 . "\n";

exit 0;
