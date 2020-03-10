#!/usr/bin/env perl
#
# Copyright (C) 2017 Kurt Kanzenbach <kurt@linutronix.de>
# Time-stamp: <2017-07-28 12:34:11 kurt>
#
# Example Script: Show howto check dataIn signatures using OpenSSL.
#

use strict;
use warnings;
use Carp;
use XML::LibXML;
use MIME::Base64 qw(encode_base64);
use File::Temp qw(tempfile);

sub get_input
{
    my ($line, $input);

    $input = "";
    while ($line = <>) {
        $input .= $line;
    }

    croak "No input given" if $input eq q{};

    return $input;
}

sub check_signature
{
    my ($input) = @_;
    my ($parser, $output, $dom, $fh, $xml_file, $sig_orig, $sig_gen, $sig_child);

    # parse XML and remove signature argument
    $parser = XML::LibXML->new();
    $dom = $parser->load_xml(string => $input);
    $dom->setEncoding("UTF-8");

    $sig_child = ($dom->documentElement()->getChildrenByTagName("signature"))[0];
    $sig_orig = $sig_child->textContent;
    croak "No signature found in config XML" unless $sig_child;
    $dom->documentElement()->removeChild($sig_child);
    ($fh, $xml_file) = tempfile();
    print $fh $dom->toString(1);
    close $fh;

    $output = `openssl dgst -sha1 -hmac 20:cd:39:b8:c5:2a $xml_file`;
    croak "Executing openssl failed" if $?;
    croak "Failed to generated signature\n" unless (($sig_gen) = $output =~ /.*?\= ([a-f0-9]+)/);
    print $sig_gen . "\n";

    # encode base64
    $sig_gen = encode_base64($sig_gen);

    # cleanup
    unlink $xml_file;

    # check
    print "orig=$sig_orig\n";
    print "gen=$sig_gen\n";
    return $sig_gen eq $sig_orig;
}

sub main
{
    my ($req, $ret);

    $req = get_input();
    $ret = check_signature($req);

    print "Signature check successfull\n" if $ret;
    print "Signature not check successfull\n" unless $ret;

    return;
}

main();

exit 0;
