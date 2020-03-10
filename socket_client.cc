//-----------------------------------------------------------------------------
///
/// \brief  Socket client
///
///         Provides client side for libzix commands
///
/// \date   [20161219] File created
///
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


#include "socket_client.h"
#include "socket_interface_connection.h"
#include "gio_istream_adapter.h"
#include "gio_istream_adapter.h"
#include "byteslist_istream.h"
#include "xml_result_parsed.h"
#include "xml_node_parameter.h"


//---Implementation------------------------------------------------------------


SocketClient::SocketClient( int port )
    : Glib::ObjectBase (typeid (SocketClient))
    //:InterfaceHandler("Client", xml_processor)
{
    _client = Gio::SocketClient::create();
    try
    {
        _connection = _client->connect_to_host("localhost:17001", port);
    }
    catch ( ... )
    {
        printf("Could not connect to server\n");
        throw std::logic_error("Could not connect to server");
    }

    printf ("socket interface created\n");
}


Glib::RefPtr <SocketClient>
SocketClient::create (int port)
{
    return Glib::RefPtr <SocketClient> (new SocketClient(port) );
}


SocketClient::~SocketClient()
{
}


void
SocketClient::incoming_data ( Gio::SocketClientEvent,
                              const Glib::RefPtr<Gio::SocketConnectable>&,const Glib::RefPtr<Gio::IOStream>& )
{
    printf("incoming data\n");
}


void SocketClient::stream_complete (std::list <Glib::RefPtr <Glib::Bytes> > bytes_list)
{
    (void)bytes_list;
    printf("incoming data (stream completed)\n");
}


int SocketClient::sendRequest(const Glib::ustring &request, Glib::RefPtr<XmlResult> &result)
{
    auto output_stream = _connection->get_output_stream ();
    const unsigned char* contents;
    gsize bytes_count;

    //printf("Sending data: %s\n", request.c_str());
    output_stream->write (request);
    output_stream->write ( "\0", 1);
    output_stream->flush();

    Glib::RefPtr< Gio::Cancellable > cancellable;
    Glib::RefPtr <Glib::Bytes> bytesList=_connection->get_input_stream()->read_bytes( 0x1000, cancellable );

    xmlpp::DomParser _parser;
    //_parser.parse_stream( bytesListStream );
    contents = (const unsigned  char *)bytesList->get_data( bytes_count );
    printf("Raw: %s\n", contents);
    _parser.parse_memory_raw( contents, bytes_count );

    result=XmlResultParsed::create(contents, bytes_count);

    return( result->get_status() );
}


int SocketClient::sendRequest(const Glib::ustring &request, Glib::ustring &response)
{
    int sta;
    Glib::RefPtr<XmlResult> result;

    sta=sendRequest(request, result);

    //result->dump();
    //result->dump_params();
    const XmlParameterList &list=result->getParameterList();
    if(list.size())
    {
        Glib::RefPtr<XmlNodeParameter> node=list.get_first < XmlNodeParameter > ();
        printf("A\n");
        if( node )
        {
            //printf("Yeaaah!\n");
            response=node->to_xml();
        }
    }

    return( sta );
}


/*
int SocketClient::getConf(const Glib::ustring &resource
                          , const Glib::ustring &parameter
                          , Glib::ustring &value)
{
    (void)resource;
    (void)value;
    (void)parameter;
    int sta=0;
    Glib::ustring response;

    Glib::RefPtr<XmlResult> result=sendRequest( Glib::ustring::compose(
                         //"<function fid=\"%1\"><resource id=\"%2\" />"
                         //"<function fid=\"%1\" host=\"SIC\" id=\"/tmp/update.sh\">\n"
                         //"</function>\n"
                         "<function fid=\"getConf\" id=\"USB:folder\" />"
                         , "getConf", resource), response );

    printf("Response: %s\n", response.c_str());

    printf("Response: %s\n", result->to_xml().c_str());

    sta=result->get_status();

    return(sta);
}
*/

int SocketClient::getConf(const Glib::ustring &item
                          , const Glib::ustring &resource
                          , const Glib::ustring &node
                          , const Glib::ustring &postprocessor
                          , Glib::ustring &response)
{
    int sta=0;

    Glib::ustring request=Glib::ustring::compose(
                "<function fid=\"%1\" item=\"%2\" ", "getConf", item);
    if(!resource.empty())
        request+=Glib::ustring::compose("resource=\"%1\" ", resource);
    if(!resource.empty())
        request+=Glib::ustring::compose("node=\"%1\" ", node);
    if(!resource.empty())
        request+=Glib::ustring::compose("postprocessor=\"%1\" ", postprocessor);

    request+="/>";

    sta=sendRequest( request, response );

    return(sta);
}


int SocketClient::getConfNetwork( bool &dhcp
                          , Glib::ustring &ip
                          , Glib::ustring &subnetMask
                          , Glib::ustring &dns1
                          , Glib::ustring &dns2
                          , Glib::ustring &gateway
                          , Glib::ustring &ntp )
{
    int sta=0;

    (void)dhcp;
    (void)ip;
    (void)subnetMask;
    (void)dns1;
    (void)dns2;
    (void)gateway;
    (void)ntp;
    Glib::RefPtr<XmlResult> result;
    sta=sendRequest( "<function fid=\"getConf\" item=\"parameter\" node=\".//interface[id='lan']\" />", result );

    printf("Response status: %d\n", sta );
    printf("Response parameters: %d\n", (int)result->getParameterList().size() );
    result->getParameterList().dump();

    Glib::RefPtr<XmlNodeParameter> node=result->getParameterList().get_first < XmlNodeParameter > ();
    if( node )
    {
        printf( "Yeaaah!\n");
        printf( "Text: %s\n", node->to_xml().c_str() );
    }

    return(sta);
}


int SocketClient::setFile(  const Glib::ustring &host
                          , const Glib::ustring &filename
                          , const Glib::ustring &data)
{
    int sta=0;

    (void)host;
    (void)filename;
    (void)data;
    Glib::RefPtr<XmlResult> result;

    Glib::ustring request=
            Glib::ustring::compose(
        "<function fid=\"setFile\">\n"
            "<signature>672687268714...</signature>\n"
            "<file host=\"%1\" id=\"%2\" type=\"binary\">\n"
                "SGFsbG8gV2VsdCEK\n"
            "</file>\n"
        "</function>\n", host, filename);

    sta=sendRequest( request, result );

    printf("Response status: %d\n", sta );
    printf("Response parameters: %d\n", (int)result->getParameterList().size() );
    result->getParameterList().dump();

    return(sta);
}


//---fin.---------------------------------------------------------------------
