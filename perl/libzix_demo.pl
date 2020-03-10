#!/usr/bin/env perl
#
# Copyright (C) 2016 Kurt Kanzenbach <kurt@linutronix.de>
# Time-stamp: <2017-01-17 13:44:41 kurt>
#
# This script demonstrates howto use the Perl LibZIX client.
#
# The LibZIX Perl client provides basically two mechanisms in order to access
# the LibZIX library functions. One basic API where the request and response are
# simply passed as strings to the request() function. The client then simply
# performs the request and returns the result.  The second way is, the LibZIX
# Perl client provides wrapper subroutines for all LibZIX functions. These
# function take specific arguments and return a unified result.  Each function
# is documented in LibZIX::SocketClient.pm. Communication between LibZIX daemon
# and LibZIX::SocketClient.pm is done by a TCP socket. However, it can be done
# via UNIX domain sockets as well. See examples below.
#
# Arguments always contains the parameters for <function> call and the
# body. Some functions take a specific body (e.g. setFile). The body is simply
# passed as XML string. The reason for that, the function body differs for
# different function calls.
#
# The return value for all function call is a hash reference containing:
#  - status  - contains result status e.g. 200
#  - message - [optional] result message, esp. interesting of status != 200
#  - body    - [optional] as string, the body can be passed to LibXML in order to
#              retrieve the function call specific return values (depends on fid)
#
# This script demonstrates how to call and use the LibZIX library
# functions. This covers performing simple requests and parsing the responses as
# well as performing signed function calls.
#

use strict;
use warnings;
use FindBin;
use lib "$FindBin::RealBin/";
use LibZIX::SocketClient;
use Carp;

# TCP Socket settings:
#  Defaults to localhost @ port 17001
my ($host, $port) = ("localhost", 17001);

sub test_request
{
    my ($zix_client, $xml_req, $ref);

    print "-" x 80 . "\n";
    print "TEST: test_request\n";

    # build request: in this example -> plain string,
    # but can also be constructed via LibXML (e.g. see LibZIX::SocketClient::func_wrapper)
    $xml_req = << 'EOF';
<?xml version="1.0"?>
<function fid="getFilesList" host="SIC" dir="/" />
EOF

    # performs standard request
    $zix_client = LibZIX::SocketClient->new(host => $host, port => $port)
        or croak "Failed to create socket client: $!";
    $ref = $zix_client->request($xml_req)
        or croak "Failed to perform request: $!";
    $zix_client->close_socket();

    #
    # the result is a hash reference:
    #
    print "Status: "  . $ref->{status}  . "\n";
    print "Message: " . $ref->{message} . "\n" if $ref->{message};
    print "Body: "    . $ref->{body}    . "\n" if $ref->{body};
    print "-" x 80 . "\n";

    return;
}

sub test_signed_request
{
    my ($zix_client, $xml_req, $ref);

    print "-" x 80 . "\n";
    print "TEST: test_signed_request\n";

    # If you want to perform a signed request, you _have to_ add
    # signature tag with some bogues value. For a detailed description,
    # see LibZIX::SocketClient::signed_request.
    $xml_req = << 'EOF';
<?xml version="1.0"?>
<function fid="getFilesList" host="SIC" dir="/">
  <signature>foo</signature>
</function>
EOF

    # performs signed request
    $zix_client = LibZIX::SocketClient->new
        or croak "Failed to create socket client: $!";
    $ref = $zix_client->signed_request($xml_req)
        or croak "Failed to perform request: $!";
    $zix_client->close_socket();

    print "Status: "  . $ref->{status}  . "\n";
    print "Message: " . $ref->{message} . "\n" if $ref->{message};
    print "Body: "    . $ref->{body}    . "\n" if $ref->{body};
    print "-" x 80 . "\n";

    return;
}

sub test_get_conf
{
    my ($zix_client, $ref);

    print "-" x 80 . "\n";
    print "TEST: test_get_conf\n";
    $zix_client = LibZIX::SocketClient->new
        or croak "Failed to create socket client: $!";

    $ref = $zix_client->getConf(item => "parameter", id => "serialnumber")
        or croak "Failed to perform request: $!";
    $zix_client->close_socket;

    print "Status: "  . $ref->{status}  . "\n";
    print "Message: " . $ref->{message} . "\n";
    print "Result: "  . $ref->{body}    . "\n";
    print "-" x 80 . "\n";

    return;
}

sub test_get_conf_signed
{
    my ($zix_client, $ref);

    print "-" x 80 . "\n";
    print "TEST: test_get_conf_signed\n";
    $zix_client = LibZIX::SocketClient->new
        or croak "Failed to create socket client: $!";

    # NOTE: the signed option can be passed to every $zix_client function
    $ref = $zix_client->getConf(item => "parameter", signed => 1, id => "serialnumber")
        or croak "Failed to perform request: $!";
    $zix_client->close_socket;

    print "Status: "  . $ref->{status}  . "\n";
    print "Message: " . $ref->{message} . "\n" if $ref->{message};
    print "Result: "  . $ref->{body}    . "\n";
    print "-" x 80 . "\n";

    return;
}

sub test_get_conf_unix_socket
{
    my ($zix_client, $ref);

    print "-" x 80 . "\n";
    print "TEST: test_get_conf_unix_socket\n";

    #
    # Difference: This time, the zix client will use a unix domain socket,
    # instead of a network socket.
    #
    $zix_client = LibZIX::SocketClient->new_unix(path => "/path/to/socket")
        or croak "Failed to create unix socket client: $!";

    $ref = $zix_client->getConf(item => "parameter", id => "serialnumber")
        or croak "Failed to perform request: $!";
    $zix_client->close_socket;

    print "Status: "  . $ref->{status}  . "\n";
    print "Message: " . $ref->{message} . "\n";
    print "Result: "  . $ref->{body}    . "\n";
    print "-" x 80 . "\n";

    return;
}

sub test_set_conf
{
    my ($zix_client, $ref);

    print "-" x 80 . "\n";
    print "TEST: test_set_conf\n";
    $zix_client = LibZIX::SocketClient->new
        or croak "Failed to create socket client: $!";

    $ref = $zix_client->setConf(
        body => '<parameter id="serialBaudrate" value="115200" unit="Bd"/>',
        mode => "normal")
        or croak "Failed to perform request: $!";
    $zix_client->close_socket;

    print "Status: "  . $ref->{status}  . "\n";
    print "Message: " . $ref->{message} . "\n" if $ref->{message};
    print "-" x 80 . "\n";

    return;
}

sub test_log
{
    my ($zix_client, $ref);

    print "-" x 80 . "\n";
    print "TEST: test_log\n";
    $zix_client = LibZIX::SocketClient->new
        or croak "Failed to create socket client: $!";

    $ref = $zix_client->log(category => "Service", level => "Warning",
                            timestamp => "2016-11-19 19:31:02.154",
                            source => "DC", theme => "Device UV calibration",
                            message => "Test Logging...")
        or croak "Failed to perform request: $!";
    $zix_client->close_socket;

    print "Status: "  . $ref->{status}  . "\n";
    print "Message: " . $ref->{message} . "\n" if $ref->{message};
    print "-" x 80 . "\n";

    return;
}

sub test_data_sync
{
    my ($zix_client, $ref, $measurement_xml);

    print "-" x 80 . "\n";
    print "TEST: test_data_sync\n";
    $zix_client = LibZIX::SocketClient->new
        or croak "Failed to create socket client: $!";

    $ref = $zix_client->dataSync(type => "NUM", iid => "00002",
                                 body => '<measurement iid="00002" format="generic" />')
        or croak "Failed to perform request: $!";
    $zix_client->close_socket;

    print "Status: "  . $ref->{status}  . "\n";
    print "Message: " . $ref->{message} . "\n" if $ref->{message};
    print "-" x 80 . "\n";

    return;
}

sub test_data_out
{
    my ($zix_client, $ref);

    print "-" x 80 . "\n";
    print "TEST: test_data_out\n";
    $zix_client = LibZIX::SocketClient->new
        or croak "Failed to create socket client: $!";

    $ref = $zix_client->dataOut(type => "measurement", iid => "00002",
                                dest => "lanFolder", format => "PDF enhanced",
                                output => "3")
        or croak "Failed to perform request: $!";
    $zix_client->close_socket;

    print "Status: "  . $ref->{status}  . "\n";
    print "Message: " . $ref->{message} . "\n" if $ref->{message};
    print "-" x 80 . "\n";

    return;
}

sub test_data_free
{
    my ($zix_client, $ref);

    print "-" x 80 . "\n";
    print "TEST: test_data_free\n";
    $zix_client = LibZIX::SocketClient->new
        or croak "Failed to create socket client: $!";

    $ref = $zix_client->dataFree(iid => "00002", enforceDeletion => "yes")
        or croak "Failed to perform request: $!";
    $zix_client->close_socket;

    print "Status: "  . $ref->{status}  . "\n";
    print "Message: " . $ref->{message} . "\n" if $ref->{message};
    print "-" x 80 . "\n";

    return;
}

sub test_get_files_list
{
    my ($zix_client, $ref);

    print "-" x 80 . "\n";
    print "TEST: test_get_files_list\n";
    $zix_client = LibZIX::SocketClient->new
        or croak "Failed to create socket client: $!";

    $ref = $zix_client->getFilesList(host => "SIC", dir => "/")
        or croak "Failed to perform request: $!";
    $zix_client->close_socket;

    # return value is a hash ref
    print "Status: "  . $ref->{status}  . "\n";
    print "Message: " . $ref->{message} . "\n" if $ref->{message};
    print "Files: "   . $ref->{body}    . "\n";
    print "-" x 80 . "\n";

    return;
}

sub test_get_file
{
    my ($zix_client, $ref);

    print "-" x 80 . "\n";
    print "TEST: test_get_file\n";
    $zix_client = LibZIX::SocketClient->new
        or croak "Failed to create socket client: $!";

    $ref = $zix_client->getFile(host => "SIC",
                                body => '<file id="/usr/bin/yes" />
                                         <file id="zix.log" />')
        or croak "Failed to perform request: $!";
    $zix_client->close_socket;

    print "Status: "  . $ref->{status}  . "\n";
    print "Message: " . $ref->{message} . "\n" if $ref->{message};
    print "Files: "   . $ref->{body}    . "\n";
    print "-" x 80 . "\n";

    return;
}

sub test_set_file
{
    my ($zix_client, $ref);

    print "-" x 80 . "\n";
    print "TEST: test_set_file\n";
    $zix_client = LibZIX::SocketClient->new
        or croak "Failed to create socket client: $!";

    $ref = $zix_client->setFile(body => '<file id="test_set_file">Base64encoded</file>',
                                host => "SIC")
        or croak "Failed to perform request: $!";
    $zix_client->close_socket;

    print "Status: "  . $ref->{status}  . "\n";
    print "Message: " . $ref->{message} . "\n" if $ref->{message};
    print "-" x 80 . "\n";

    return;
}

sub test_del_file
{
    my ($zix_client, $ref);

    print "-" x 80 . "\n";
    print "TEST: test_del_file\n";
    $zix_client = LibZIX::SocketClient->new
        or croak "Failed to create socket client: $!";

    $ref = $zix_client->delFile(host => "SIC",
                                body => '<file id="test_set_file" />')
        or croak "Failed to perform request: $!";
    $zix_client->close_socket;

    print "Status: "  . $ref->{status}  . "\n";
    print "Message: " . $ref->{message} . "\n" if $ref->{message};
    print "-" x 80 . "\n";

    return;
}

test_request();
test_get_conf();
test_set_conf();
test_log();
test_data_sync();
test_data_out();
test_data_free();
test_get_files_list();
test_get_file();
test_set_file();
test_del_file();
# test_get_conf_unix_socket(); Path of socket needs to be defined, before running it.
test_get_conf_signed();
test_signed_request();

exit 0;
