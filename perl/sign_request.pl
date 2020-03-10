#!/usr/bin/env perl
#
# Copyright (C) 2017 Kurt Kanzenbach <kurt@linutronix.de>
# Time-stamp: <2017-01-18 16:28:22 kurt>
#
# This script can be used to generate signed XML requests.  A XML requests is
# read by stdin and the signed one is printed to stdout.
#
# For Documentation of signing XML requests, please refer to
# LibZIX::SocketClient::signed_request.
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

sub sign_it
{
    my ($input) = @_;
    my ($parser, $output, $sock, $line, $dom, $fh, $xml_file, $gpg_file, $sig,
        $sig_child);

    # parse XML and remove signature argument
    $parser = XML::LibXML->new();
    $dom = $parser->load_xml(string => $input);
    $dom->setEncoding("UTF-8");

    $sig_child = ($dom->documentElement()->getChildrenByTagName("signature"))[0];
    croak "If you use signed_request the XML _must_ have a signature tag"
        unless $sig_child;
    $dom->documentElement()->removeChild($sig_child);
    ($fh, $xml_file) = tempfile();
    print $fh $dom->toString(1);
    close $fh;

    # create gpg signature
    # FIXME: Keysettings needs to be defined!
    ($fh, $gpg_file) = tempfile();
    close $fh;
    unlink $gpg_file;
    `gpg --batch --yes --output $gpg_file --detach-sig $xml_file >/dev/null 2>/dev/null`;
    croak "Executing gpg failed" if $?;

    # encode base64
    open($fh, "<", $gpg_file) || croak "Failed to open file $gpg_file: $!";
    $sig = do { local $/ = undef ; <$fh> };
    close $fh;
    $sig = encode_base64($sig);

    # cleanup
    unlink $gpg_file;
    unlink $xml_file;

    # add signature tag with correct value (parse XML again and replace signature content)
    $parser = XML::LibXML->new();
    $dom = $parser->load_xml(string => $input);
    $dom->setEncoding("UTF-8");

    $sig_child = ($dom->documentElement()->getChildrenByTagName("signature"))[0];
    $sig_child->removeChildNodes();
    $sig_child->appendText($sig);

    return $dom->toString(1);
}

sub main
{
    my ($req, $out);

    $req = get_input();
    $out = sign_it($req);

    print "$out";

    return;
}

main();

exit 0;
