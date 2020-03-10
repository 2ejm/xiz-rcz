#!/usr/bin/env perl
#
# Copyright (C) 2016 Kurt Kanzenbach <kurt@linutronix.de>
# Time-stamp: <2016-12-15 10:56:46 kurt>
#

use strict;
use warnings;
use XML::LibXML;

# configuration
my ($svg_template, $svg_out, $rsvg);

$svg_template = "template.svg";
$svg_out      = "demo.svg";
$rsvg         = "/usr/bin/rsvg-convert";

# args
my ($xml_file, $pdf_file);

# internal data
my (
    %data,                      # Used for replacement inside template file
    %template_data              # Contains needed information for drawing curves inside the svg
   );

%template_data = (
    dx => 10083,                # width of coordinate system inside svg template
    dy => 3813,                 # height of coordinate system inside svg template
    startx => 350,              # x -> start offset in coordinate system
    starty => 0,                # y -> start offset in coordinate system
    id_right => "id177",        # id of right graph (corresponds to <g id="...")
    id_left => "id187",         # id of left graph (corresponds to <g id="...")
    id_rightp => "id178",       # id where to draw points of right graph
    id_leftp => "id191",        # id where to draw points of left graph
    rgb_rightp => "rgb(31,73,125)", # point color for right curve
    rgb_leftp => "rgb(0,176,80)",   # point color for left curve
   );

sub _err
{
    my ($msg) = @_;
    my (undef, $file, $line, undef) = caller(0);

    chomp $msg;
    print STDERR "[ERROR $file:$line]: $msg\n";

    cleanup();

    exit -1;
}

sub print_usage_and_die
{
    print STDERR "usage: $0 <xml_file>.xml <pdf_file>.pdf\n";
    exit -1;
}

sub get_args
{
    ($xml_file, $pdf_file) = @ARGV;
    print_usage_and_die() unless defined $xml_file;
    print_usage_and_die() unless defined $pdf_file;

    print_usage_and_die() unless $xml_file =~ /\.xml$/;
    print_usage_and_die() unless $pdf_file =~ /\.pdf$/;

    unless (-x $rsvg) {
        print STDERR "rsvg-convert binary not found\n";
        exit -1;
    }

    unless (-e $xml_file) {
        print STDERR "Input XML file does not exist\n";
        exit -1;
    }

    return;
}

sub parse_measurement_xml
{
    my ($parser, $doc);

    $parser = XML::LibXML->new();
    eval {
        $doc = $parser->parse_file($xml_file);
    };
    _err("Failed to parse XML file $xml_file") if ($@);

    # TODO: read values from measurement into %data
    $data{NAME} = "demo";

    return;
}

sub get_db_data
{
    # TODO: Connect to patient database, gather values and store them into %data
    return;
}

sub create_svg
{
    my ($fh_template, $fh_actual, $line);

    open($fh_template, "<", $svg_template) || _err("Failed to open file $svg_template: $!");
    open($fh_actual,   ">", $svg_out)      || _err("Failed to open file $svg_out: $!");

    while ($line = <$fh_template>) {
        foreach my $key (keys %data) {
            my $value = $data{$key};
            $line =~ s/\$\{\Q$key\E\}/$value/g;
        }
        print $fh_actual $line;
    }

    close $fh_template;
    close $fh_actual;

    return;
}

sub create_pdf
{
    `$rsvg -f pdf -o $pdf_file $svg_out`;

    _err("$rsvg failed: $?") if ($?);

    return;
}

sub do_points_transformation
{
    my ($arr_ref) = @_;

    for (my $i = 0; $i < @{$arr_ref}; $i += 2) {
        my ($x, $y) = ($arr_ref->[$i], $arr_ref->[$i + 1]);

        $x -= $template_data{startx};
        $y -= $template_data{starty};
        $x = $x * $template_data{dx} / 100;

        $y = $y * $template_data{dy} / 100;

        $arr_ref->[$i + 0] = $x;
        $arr_ref->[$i + 1] = $y;
    }

    return;
}

sub build_curve_svg_path
{
    my ($arr_ref) = @_;
    my ($path);

    $path = "M 0,0";
    for (my $i = 0; $i < @{$arr_ref}; $i += 2) {
        my ($x, $y) = ($arr_ref->[$i], $arr_ref->[$i + 1]);
        $path .= " L $x,$y";
    }

    return $path;
}

sub add_points_to_svg
{
    my ($node, $arr_ref, $rgb_key) = @_;

    for (my $i = 0; $i < @{$arr_ref}; $i += 2) {
        my ($circle);
        my ($x, $y) = ($arr_ref->[$i], $arr_ref->[$i + 1]);

        $circle = XML::LibXML::Element->new("circle");

        $circle->setAttribute("cx", $x);
        $circle->setAttribute("cy", $y);
        $circle->setAttribute("r", "130");
        $circle->setAttribute("stroke", $template_data{$rgb_key});
        $circle->setAttribute("stroke-width", "35");
        $circle->setAttribute("fill", "rgb(255,255,255)");
        $node->addChild($circle);
    }

    return;
}

sub place_curves_inside_template
{
    my ($parser, $doc, $xc, @nodes, @right_curve, @left_curve,
        $right_path, $left_path);

    $parser = XML::LibXML->new();
    eval {
        $doc = $parser->parse_file($svg_out);
    };
    _err("Failed to parse svg XML in $svg_out") if ($@);

    # find right eye curve and replace it
    $xc = XML::LibXML::XPathContext->new($doc);
    $xc->registerNs('svg', 'http://www.w3.org/2000/svg');
    @nodes = $xc->findnodes("//svg:g[\@id='$template_data{id_right}']/svg:path");

    _err("Failed to find right curve path") unless @nodes == 1;

    # example curves!
    @right_curve = (
        360, 10,
        375, 40,
        400, 60,
        425, 50,
        440, 70,
       );
    @left_curve = (
        360, 20,
        375, 30,
        400, 80,
        425, 20,
        440, 60,
       );

    # transform into right coordinate system
    do_points_transformation(\@right_curve);
    do_points_transformation(\@left_curve);

    # replace graphs
    $nodes[0]->setAttribute("d", build_curve_svg_path(\@right_curve));

    @nodes = $xc->findnodes("//svg:g[\@id='$template_data{id_left}']/svg:path");
    _err("Failed to find left curve path") unless @nodes == 1;

    $nodes[0]->setAttribute("d", build_curve_svg_path(\@left_curve));

    # replace points
    @nodes = $xc->findnodes("//svg:g[\@id='$template_data{id_leftp}']");
    _err("Failed to find left points id") unless @nodes == 1;
    add_points_to_svg($nodes[0], \@left_curve, "rgb_leftp");

    @nodes = $xc->findnodes("//svg:g[\@id='$template_data{id_rightp}']");
    _err("Failed to find right points id") unless @nodes == 1;
    add_points_to_svg($nodes[0], \@right_curve, "rgb_rightp");

    # write new document
    $doc->toFile($svg_out, "0");

    return;
}

sub cleanup
{
    unlink $svg_out if -e $svg_out;
    return;
}

sub main
{
    get_args();
    parse_measurement_xml();
    get_db_data();
    create_svg();
    place_curves_inside_template();
    create_pdf();
    cleanup();
}

main();

exit 0;
