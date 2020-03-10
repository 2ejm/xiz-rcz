#
# LibZIX::SocketClient
#
# Copyright (C) 2016 Kurt Kanzenbach <kurt@linutronix.de>
# Time-stamp: <2017-01-25 09:13:24 kurt>
#

package LibZIX::SocketClient;

use strict;
use warnings;
use Exporter;
use Carp;
use IO::Socket::INET;
use IO::Socket::UNIX;
use XML::LibXML;
use MIME::Base64 qw(encode_base64);
use File::Temp qw(tempfile);

our $VERSION = "1.00";

our ($XML_STATUS_OK, $XML_STATUS_BAD_REQUEST, $XML_STATUS_INTERNAL_DEVICE_ERROR,
     $XML_STATUS_NOT_FOUND, $XML_STATUS_FORBIDDEN, $XML_STATUS_TOO_MANY_REQUESTS) =
    (200, 400, 500, 404, 403, 429);

#
# This sub creates a new LibZIX socket client. Currently a TCP socket client.
#
# params:
#  - host -> hostname or ip address for socket connection
#  - port -> port for socket connection
#  - path -> path to unix domain socket
#
# return:
#  -> reference to object
#
sub new
{
    my ($class, %arg) = @_;

    return new_tcp($class, %arg);
}

#
# This sub creates a new TCP LibZIX socket client.
#
# params:
#  - host -> hostname or ip address for socket connection
#  - port -> port for socket connection
#
# return:
#  -> reference to object
#
sub new_tcp
{
    my ($class, %arg) = @_;
    my ($self, $sock);

    $sock = IO::Socket::INET->new(PeerAddr => $arg{host} // q{localhost},
                                  PeerPort => $arg{port} // q{17001},
                                  Proto => q{tcp})
        or return;

    $sock->autoflush();
    $self = { sock => $sock };

    bless $self, $class;

    return $self;
}

#
# This sub creates a new UNIX LibZIX socket client.
#
# params:
#  - path -> path to unix domain socket
#
# return:
#  -> reference to object
#
sub new_unix
{
    my ($class, %arg) = @_;
    my ($self, $sock);

    croak "No path given for unix domain socket" unless exists $arg{path};

    $sock = IO::Socket::UNIX->new(Peer => $arg{path},
                                  Type => SOCK_STREAM())
        or return;

    $sock->autoflush();
    $self = { sock => $sock };

    bless $self, $class;

    return $self;
}

#
# This functions closes the underlying socket. This sub should be called if the
# client object isn't needed anymore. After close_socket() don't call any further
# functions on this object.
#
# params:
#  - none
#
# return:
#  -> none
#
sub close_socket
{
    my ($self) = @_;

    $self->{sock}->close();

    return;
}

#
# Takes a XML request, sends it to libzix and receives the response.  This sub
# can be used directly by the caller, but doesn't have to be. This module
# provides user-friendly subs for most common libzix functions as well. Try to
# use the wrappers instead of this one.
#
# params:
#  - req -> XML request as string
#
# return:
#  -> xml response as string
#
sub request_raw
{
    my ($self, $req) = @_;
    my ($output, $sock, $line);

    eval {
        $sock = $self->{sock};
        print $sock $req;
        print $sock "\0";

        $output = "";
        while (1) {
            $sock->recv($line, 1024);
            $output .= $line;
            last if $line =~ /\0/;
        }
        chop $output;
    };

    return if $@;

    return $output;
}

#
# This subroutines does the same as the one sub, but prepares the result
# into status, message and response body. This way the caller can immediatly
# compare the status and so on.
#
# params:
#  - req -> XML request as string
#
# return:
#  -> xml result hash (keys: status, message, body)
#
sub request
{
    my ($self, $req) = @_;
    my ($output);

    $output = $self->request_raw($req) or return;

    return $self->parse_xml_result($output);
}

#
# FIXME: We need to define the *KEY SETTINGS*. Where does the private key come from?
#        Where is it stored? Which one to use?
#
# This sub has the same behaviour, arguments and return values as
# request(). There is only one exception: It signs the request with a signature.
#
# A signed XML request _must_ have a signature tag defined with some bogus value.
#
# Example:
#
#  <function fid="getFilesList" id="/bin" host="SIC">
#    <signature>foo</signature>
#  </function>
#
# This subroutine will create the signature for the given request and will store
# it (base64 encoded) inside the signature tag. The "finalized" request is passed
# over to request subroutine.
#
# Note: The whitespace handling for generating the signature of the XML request is
#       very important and has to stay consistent with the libzix implementation.
#       That's why you shouldn't do it your self, this function takes care of you.
#
sub signed_request
{
    my ($self, $req) = @_;
    my ($parser, $output, $sock, $line, $dom, $fh, $xml_file, $gpg_file, $sig,
        $sig_child);

    # parse XML and remove signature argument
    $parser = XML::LibXML->new();
    $dom = $parser->load_xml(string => $req);
    $dom->setEncoding("UTF-8");

    $sig_child = ($dom->documentElement()->getChildrenByTagName("signature"))[0];
    croak "If you use signed_request the XML _must_ have a signature tag"
        unless $sig_child;
    $dom->documentElement()->removeChild($sig_child);
    ($fh, $xml_file) = tempfile();
    print $fh $dom->toString(1);
    close $fh;

    # create gpg signature
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
    $dom = $parser->load_xml(string => $req);
    $dom->setEncoding("UTF-8");

    $sig_child = ($dom->documentElement()->getChildrenByTagName("signature"))[0];
    $sig_child->removeChildNodes();
    $sig_child->appendText($sig);

    return $self->request($dom->toString(1));
}

#
# Subroutine used to extract $status, $message and $body from an XML
# result.
#
# params:
#  - output -> response xml as string
#
# return:
#  - hash ref (keys: status, message and body)
#
sub parse_xml_result
{
    my ($self, $output) = @_;
    my ($parser, $dom, $status, $message, @nodes, $body);

    eval {
        $parser = XML::LibXML->new();
        $dom    = $parser->load_xml(string => $output);
        $status = $dom->documentElement()->getAttribute("status");
        croak "Received mal formed XML result" unless $status;

        @nodes = $dom->documentElement()->getChildrenByTagName("message");
        $message = $nodes[0] ? $nodes[0]->textContent : "";

        @nodes = $dom->documentElement()->getChildNodes();
        $body = "";
        foreach my $node (@nodes) {
            next if $node->nodeName eq "message";
            $body .= $node->toString(1);
        }
    };

    return if $@;

    return { status => $status, message => $message, body => $body };
}

#
# This subroutine wraps the LibZIX function calls. It takes an hash containing
# the xml parameters and the xml function body, builds the XML request and performs
# the function call.
#
# param:
#  - params -> array ref of hash refs (keys: value, name)
#  - body   -> function body (will be enclosed in <function $parameters>$body</function>)
#  - signed -> signed request?
#
# return:
#  - hash ref (keys: status, message and body)
#
sub func_wrapper
{
    my ($self, %arg) = @_;
    my ($doc, $root, $dom, $elem);

    # new function
    $doc = XML::LibXML::Document->new("1.0", "UTF-8");
    $root = $doc->createElement("function");

    # add parameters
    foreach my $par (@{ $arg{params} }) {
        $root->setAttribute($par->{name} => $par->{value});
    }

    # add body
    if ($arg{body}) {
        $dom = XML::LibXML->new()->load_xml(
            string => "<dummyroot>$arg{body}</dummyroot>");
        foreach my $node ($dom->documentElement()->childNodes()) {
            $root->addChild($node);
        }
    }

    # want signature?
    if ($arg{signed}) {
        $elem = $doc->createElement("signature");
        $elem->appendTextNode("foo");
        $root->addChild($elem);
    }

    # finish off
    $doc->setDocumentElement($root);

    # do it
    return $arg{signed} ? $self->signed_request($doc->toString(1)) :
        $self->request($doc->toString(1));
}

#
# This subroutine is used to construct %data hash for func_wrapper. For internal
# use only.
#
# params:
#  - arg: argument hash
#
# return:
#  - data hash ref to be used to func_wrapper
#
sub prepare_arguments
{
    my ($self, $arg_ref, $fid) = @_;
    my (%data);

    $data{signed} = $arg_ref->{signed} if $arg_ref->{signed};
    $data{body}   = $arg_ref->{body}   if $arg_ref->{body};
    $data{params} = [ { name => "fid", value => "$fid" } ];

    for my $key (keys %{ $arg_ref }) {
        next if $key eq "signed";
        next if $key eq "body";
        push(@{$data{params}}, { name => "$key", value => $arg_ref->{$key} });
    }

    return \%data;
}

#
# LibZIX -- getConf
#
# params:
#  - item          -> item
#  - id            -> id
#  - resource      -> resource
#  - node          -> node
#  - postProcessor -> post processor
#  - body          -> contains <resource> tags as string
#  - signed        -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub getConf
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "getConf");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- setConf
#
# params:
#  - mode   -> mode as string
#  - node   -> xpath expression for a node
#  - value  -> value which will be assigned to the node
#  - body   -> array reference of resource XMLs as string
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub setConf
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "setConf");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- log
#
# params:
#  - category  -> category as string
#  - level     -> log level as string
#  - timestamp -> timestamp as string
#  - theme     -> theme as string
#  - source    -> DC or SIC as string
#  - message   -> log message
#  - signed    -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub log
{
    my ($self, %arg) = @_;
    my ($doc, $root, $elem, %data);

    $data{signed} = $arg{signed} if $arg{signed};
    $data{params} = [ { name => "fid", value => "log" } ];

    # add tags
    croak "category is needed for log()"  unless $arg{category};
    croak "level is needed for log()"     unless $arg{level};
    croak "timestamp is needed for log()" unless $arg{timestamp};
    croak "theme is needed for log()"     unless $arg{theme};
    croak "theme is needed for log()"     unless $arg{source};
    croak "message is needed for log()"   unless $arg{message};

    # log is special: parameters are passed inside body
    # so we'll build the body xml here

    # new function
    $doc = XML::LibXML::Document->new("1.0", "UTF-8");
    $root = $doc->createElement("function");
    $root->setAttribute("fid" => "log");

    $elem = $doc->createElement("category");
    $elem->setAttribute("value" => $arg{category});
    $root->addChild($elem);

    $elem = $doc->createElement("level");
    $elem->setAttribute("value" => $arg{level});
    $root->addChild($elem);

    $elem = $doc->createElement("timestamp");
    $elem->setAttribute("value" => $arg{timestamp});
    $root->addChild($elem);

    $elem = $doc->createElement("theme");
    $elem->setAttribute("value" => $arg{theme});
    $root->addChild($elem);

    $elem = $doc->createElement("source");
    $elem->setAttribute("value" => $arg{source});
    $root->addChild($elem);

    $elem = $doc->createElement("message");
    $elem->appendTextNode($arg{message});
    $root->addChild($elem);

    # get body xml
    $data{body} = "";
    foreach my $node ($root->childNodes()) {
        $data{body} .= $node->toString();
    }

    return $self->func_wrapper(%data);
}

#
# LibZIX -- dataSync
#
# params:
#  - iid    -> iid as string
#  - type   -> type as string
#  - body   -> for measurement this is the XML measurement, for IMG it's
#               the base64 encoded content, both as string
#  - tag    -> tag as string
#  - dap    -> database processor as string
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub dataSync
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "dataSync");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- dataOut
#
# params:
#  - iid      -> iid as string
#  - id       -> iid as string
#  - type     -> type as string
#  - format   -> format as string
#  - dest     -> dest as string
#  - output   -> output channel as string
#  - protocol -> serial protocol as string
#  - scheme   -> serial scheme as string
#  - signed   -> Want to use a signed request? Set signed to 1.
#  - body     -> Used for type conf
#
# return:
#  -> hash ref containing the parsed xml result
#
sub dataOut
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "dataOut");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- dataFree
#
# params:
#  - iid             -> iid as string
#  - enforceDeletion -> force immediate deletion
#  - signed          -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub dataFree
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "dataFree");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- dataIn
#
# params:
#  - src    -> src as string
#  - type   -> type as string
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub dataIn
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "dataIn");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- getFilesList
#
# params:
#  - host   -> host as string
#  - dir    -> dir as string
#  - src    -> src as string
#  - dest   -> dest as string
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub getFilesList
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "getFilesList");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- getFile
#
# params:
#  - host   -> host as string
#  - body   -> files tags as string
#  - src    -> src as string
#  - dest   -> dest as string
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub getFile
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "getFile");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- setFile
#
# params:
#  - host   -> host as string
#  - body   -> files tags as string
#  - src    -> src as string
#  - dest   -> dest as string
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub setFile
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "setFile");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- delFile
#
# params:
#  - host   -> host as string
#  - body   -> files tags as string
#  - src    -> src as string
#  - dest   -> dest as string
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub delFile
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "delFile");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- update
#
# params:
#  - target -> target as string
#  - method -> method ref of file ids
#  - body   -> should contain <image> tag
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub update
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "update");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- procedure
#
# params:
#  - id     -> id of procedure
#  - exec   -> number of successful procedure executions
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub procedure
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "procedure");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- guiIO
#
# params:
#  - body        -> function tags
#  - signed      -> Want to use a signed request? Set signed to 1.
#  - all other parameters are added as attributes to <function fid="guiIO" .../>
#
# return:
#  -> hash ref containing the parsed xml result
#
sub guiIO
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "guiIO");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- getMeasurementsList
#
# params:
#  - scope  -> scope
#  - dest   -> destination
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub getMeasurementsList
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "getMeasurementsList");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- getMeasurement
#
# params:
#  - id     -> external id
#  - iid    -> internal id
#  - format -> output format
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub getMeasurement
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "getMeasurement");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- setMeasurementsList
#
# params:
#  - id     -> external id
#  - src    -> src XML
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub setMeasurementsList
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "setMeasurementsList");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- setMeasurement
#
# params:
#  - id     -> external id
#  - src    -> src XML
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub setMeasurement
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "setMeasurement");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- delMeasurement
#
# params:
#  - id     -> external id
#  - scope  -> scope
#  - src    -> src XML
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub delMeasurement
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "delMeasurement");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- getCalibration
#
# params:
#  - dest   -> destination XML
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub getCalibration
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "getCalibration");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- setCalibration
#
# params:
#  - src    -> source XML
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub setCalibration
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "setCalibration");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- getDefaults
#
# params:
#  - dest   -> destination XML
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub getDefaults
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "getDefaults");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- setDefaults
#
# params:
#  - src    -> source XML
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub setDefaults
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "setDefaults");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- getProfilesList
#
# params:
#  - scope  -> scope
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub getProfilesList
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "getProfilesList");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- getProfile
#
# params:
#  - id     -> profile id
#  - dest   -> destination XML
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub getProfile
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "getProfile");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- setProfilesList
#
# params:
#  - id     -> profile id
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub setProfilesList
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "setProfilesList");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- setProfile
#
# params:
#  - id     -> profile id
#  - src    -> source XML
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub setProfile
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "setProfile");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- delProfile
#
# params:
#  - id     -> profile id
#  - src    -> source XML
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub delProfile
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "delProfile");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- getParametersList
#
# params:
#  - scope  -> scope
#  - dest   -> destination XML
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub getParametersList
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "getParametersList");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- getParameter
#
# params:
#  - id     -> id
#  - dest   -> destination XML
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub getParameter
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "getParameter");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- setParameter
#
# params:
#  - id     -> id
#  - value  -> value of parameter
#  - unit   -> unit of parameter
#  - src    -> source XML
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub setParameter
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "setParameter");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- getTemplatesList
#
# params:
#  - scope  -> scope
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub getTemplatesList
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "getTemplatesList");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- getTemplate
#
# params:
#  - id     -> template id
#  - dest   -> destination XML
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub getTemplate
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "getTemplate");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- setTemplatesList
#
# params:
#  - id     -> template id
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub setTemplatesList
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "setTemplatesList");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- setTemplate
#
# params:
#  - src    -> source XML
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub setTemplate
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "setTemplate");

    return $self->func_wrapper(%{ $ref });
}

#
# LibZIX -- delTemplate
#
# params:
#  - id     -> template id
#  - src    -> source XML
#  - body   -> function tags
#  - signed -> Want to use a signed request? Set signed to 1.
#
# return:
#  -> hash ref containing the parsed xml result
#
sub delTemplate
{
    my ($self, %arg) = @_;
    my ($ref);

    $ref = $self->prepare_arguments(\%arg, "delTemplate");

    return $self->func_wrapper(%{ $ref });
}

1;
