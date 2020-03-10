#!/usr/bin/env perl
#
# Sample bmp2jpg Perl script. This works as a wrapper for graphics magick
# convert tool.
#
# Copyright (C) 2016 Kurt Kanzenbach <kurt@linutronix.de>
# Time-stamp: <2016-12-15 11:03:32 kurt>
#

use strict;
use warnings;

my ($bmp_file, $jpg_file, $gm_tool);

$gm_tool = "/usr/bin/gm";

sub print_usage_and_die
{
    print STDERR "usage: $0 <bitmap>.bmp <jpg file>.jpg\n";
    exit -1;
}

sub get_args
{
    ($bmp_file, $jpg_file) = @ARGV;
    print_usage_and_die() unless defined $bmp_file;
    print_usage_and_die() unless defined $jpg_file;

    # check file extenions
    print_usage_and_die() unless $bmp_file =~ /\.bmp$/;
    print_usage_and_die() unless $jpg_file =~ /\.jpg$/;

    unless (-x $gm_tool) {
        print STDERR "Unable to find graphics magick gm tool\n";
        exit -1;
    }

    unless (-e $bmp_file) {
        print STDERR "Unable to find bmp $bmp_file\n";
        exit -1;
    }

    return;
}

sub bmp2jpg
{
    # using graphics magick convert tool
    `$gm_tool convert $bmp_file $jpg_file`;
    if ($?) {
        print STDERR "convert failed: $?\n";
        exit -1;
    }

    return;
}

sub main
{
    get_args();
    bmp2jpg();

    return;
}

main();

exit 0;
