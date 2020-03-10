#!/usr/bin/env perl
#
# Copyright (C) 2016 Kurt Kanzenbach <kurt@linutronix.de>
# Time-stamp: <2017-01-05 10:36:30 kurt>
#

use strict;
use warnings;
use IO::Socket::INET;
use XML::LibXML;
use Carp;
use FindBin;
use lib "$FindBin::RealBin/";
use LibZIX::SocketClient;

# configuration
my ($xml_req_dir, $xml_res_dir, $host, $port);

$xml_req_dir = "$FindBin::RealBin/../testxmls";
$xml_res_dir = "$FindBin::RealBin/../responsexmls";
$host = "localhost";
$port = "17001";

# stats
my (%stats);

# zix client
my ($zix);

$zix = LibZIX::SocketClient->new
    or croak "Failed to create socket client: $!";

sub _err
{
    my ($msg) = @_;
    my (undef, $file, $line, undef) = caller(0);

    chomp $msg;
    print STDERR "[ERROR $file:$line]: $msg\n";

    exit -1;
}

sub dir_listing
{
    my ($dir) = @_;
    my (@files, $dh, $file);

    opendir($dh, $dir) || _err("Failed to open directory $dir: $!");
    while ($file = readdir $dh) {
        next if $file =~ /^\./;
        push(@files, $file);
    }
    close $dh;

    @files = sort @files;

    return \@files;
}

sub read_file
{
    my ($file) = @_;
    my ($fh, $content);

    open($fh, "<", $file) || _err("Failed to open file $file: $!");
    $content = do { local $/; <$fh> };
    close $fh;

    return $content;
}

sub send_req_and_get_answer
{
    my ($input) = @_;
    my ($output);

    $output = $zix->request_raw($input)
        or croak "Failed to perform request: $!";

    return $output;
}

sub string2xml
{
    my ($input) = @_;
    my ($dom);

    $dom = XML::LibXML->load_xml(string => $input);

    return $dom;
}

sub remove_indent
{
    my ($str) = @_;
    my (@lines);

    @lines = split /\n/, $str;

    foreach my $line (@lines) {
        $line =~ s/^[\t ]+//g;
    }

    return join("\n", @lines);
}

sub cmp_xml
{
    my ($dom1, $dom2) = @_;
    my ($str1, $str2, @lines);

    $str1 = $dom1->toString(1);
    $str2 = $dom2->toString(1);

    $str1 = remove_indent($str1);
    $str2 = remove_indent($str2);

    return $str1 eq $str2;
}

sub perform_test
{
    my ($input_file, $response_file) = @_;
    my ($input, $output, $actual, $resp_xml, $expected_xml, $ret);

    $input  = read_file($input_file);
    $actual = send_req_and_get_answer($input);
    $output = read_file($response_file);

    $resp_xml     = string2xml($actual);
    $expected_xml = string2xml($output);

    $ret = cmp_xml($resp_xml, $expected_xml);

    unless ($ret) {
        print "Unexpected result:\n";
        print "========================================================================\n";
        print $resp_xml;
        print "========================================================================\n";
        print "Unexpected raw\n";
        print "========================================================================\n";
        print $actual;
        print "========================================================================\n";
        print "Expected result:\n";
        print "========================================================================\n";
        print $expected_xml;
        print "========================================================================\n";
    }

    return $ret;
}

sub perform_tests
{
    my ($files_ref);

    $files_ref = dir_listing($xml_req_dir);

    foreach my $file (@{ $files_ref }) {
        my ($input_file, $response_file, $ret);

        $stats{tests}++;
        $input_file = $xml_req_dir . "/" . $file;
        $response_file = $xml_res_dir . "/" . $file;

        unless (-s $response_file) {
            $stats{skipped}++;
            print "Skipping test $file b/o the expected result xml is missing.\n";
            next;
        }

        print "Starting test $file...\n";
        $ret = perform_test($input_file, $response_file);

        $stats{failed}++ unless $ret;
        $stats{success}++ if $ret;

        print "Test $file: " . ($ret ? "successful" : "failed") . "\n";
    }
}

sub print_stats
{
    $stats{tests}   = 0 unless exists $stats{tests};
    $stats{skipped} = 0 unless exists $stats{skipped};
    $stats{success} = 0 unless exists $stats{success};
    $stats{failed}  = 0 unless exists $stats{failed};

    print "-" x 80 . "\n";
    print "Tests total: "   . $stats{tests}   . "\n";
    print "Tests skipped: " . $stats{skipped} . "\n";
    print "Tests success: " . $stats{success} . "\n";
    print "Tests failed: "  . $stats{failed}  . "\n";
    print "-" x 80 . "\n";
}

perform_tests();
print_stats();

$zix->close_socket;

exit 0;
