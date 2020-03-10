//-----------------------------------------------------------------------------
///
/// \brief  USB-Serial interface handler
///
///         Handles messages through FTDI-Serial adapter.
///
///         Unlike the serial interface handler these messages are not
///         fragmented. The messages  just get an 4 Byte length header (network
///         order).
///
///         A Frame in context of this class is a complete message.
///
///         This class shares some enums with the framed serial interface
///         handler even if most of the entries dont make sense in this context.
///
/// \date   [20170130] File created
///
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

#include <stdio.h>
#include <string.h>                 // memcpy
#include <giomm/asyncresult.h>
#include <glibmm/main.h>            // signal_io
#include <fcntl.h>                  // O_RDWR
#include <iomanip>                  // std::setfill
#include <termios.h>                // speed_t, tcgetattr
#include <cassert>
#include <memory>

#include <arpa/inet.h>              // ntohl, htonl

//---Own------------------------------

#include "usbserial_interface_handler.h"
#include "serial_interface_handler.h"
#include "ustring_istream.h"
#include "conf_handler.h"           // Setting for serial port
#include "hexio.h"
#include "log.h"
#include "xml_helpers.h"
#include "procedure_step_handler.h"
#include "xml_result_timeout.h"
#include "serial_helper.h"

//---Forward declarations------------------------------------------------------


//---Implementation------------------------------------------------------------


#define USBSERIAL_RECONNECT_TIMEOUT (20*1000)

using Glib::ustring;

UsbInterfaceOffline::UsbInterfaceOffline ()
{ }

UsbInterfaceOffline::~UsbInterfaceOffline ()
{ }

const char *
UsbInterfaceOffline::what () const noexcept
{
    return "UsbInterfaceOffline";
}

UsbInterfaceHandler::UsbInterfaceHandler(ZixInterface inf, const char *deviceName, int deviceSpeed, Glib::RefPtr <XmlProcessor> xml_processor, ESerialHandlerMode _eSerialHandlerMode)
    : Glib::ObjectBase ("UsbInterfaceHandler")
    , InterfaceHandler (inf, inf.to_string(), xml_processor)
    , SerialInterface ()
    , _device_name (deviceName)
    ,eSerialHandlerMode(_eSerialHandlerMode)
    , _online (false)
{
    Glib::RefPtr <ConfHandler> conf=ConfHandler::get_instance();

    std::string strFileMode="a+";

    (void)deviceSpeed;

    // Reset the state machine
    discardReceiveFrame();

    dcTaskId=0;

    inputFileFd=open(deviceName, O_RDWR | O_NONBLOCK);
    if(inputFileFd<=0)
        throw("Could not open file");

    setPortConfiguration();

    _iochannel = Glib::IOChannel::create_from_fd( inputFileFd );
    _iochannel->set_encoding("");
    _iochannel->set_buffered(false);

    readBufferSize=USBSERIAL_BUFFER_SIZE;
    readBuffer=new gchar[readBufferSize];

    register_connection (std::shared_ptr <InterfaceConnection> (this) );

    Glib::signal_io().connect(sigc::mem_fun(*this, &UsbInterfaceHandler::OnIOCallback),
                              _iochannel, Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP);

    conf->confChanged.connect(
        sigc::mem_fun(*this, &UsbInterfaceHandler::slotConfChanged ));
    conf->confChangeAnnounce.connect(
        sigc::mem_fun(*this, &UsbInterfaceHandler::slotConfChangeAnnounce ));

    _online = true;

    lInfo("USB-Serial online\n");
};


Glib::RefPtr <UsbInterfaceHandler>
UsbInterfaceHandler::create (ZixInterface inf, const char *deviceName, int deviceSpeed, Glib::RefPtr <XmlProcessor> xml_processor, ESerialHandlerMode _eSerialHandlerMode) /* static */
{
    return Glib::RefPtr <UsbInterfaceHandler> (new UsbInterfaceHandler (inf, deviceName, deviceSpeed, xml_processor, _eSerialHandlerMode) );
};


UsbInterfaceHandler::~UsbInterfaceHandler()
{
    delete[] readBuffer;
    readBuffer=NULL;

    return;
};


void UsbInterfaceHandler::setPortConfiguration()
{
    auto baud = SerialHelper::get_baudrate_for_parameter("ipcSerialBaudrate");

    /* set the other settings (in this case, 9600 8N1) */
    struct termios settings;
    tcgetattr( inputFileFd, &settings);

    cfsetospeed(&settings, baud); /* baud rate */
    //min = 1; time = 5;
    settings.c_cflag &= ~PARENB; /* no parity */
    settings.c_cflag &= ~CSTOPB; /* 1 stop bit */
    settings.c_cflag &= ~CSIZE;
    settings.c_cflag |= CS8 | CLOCAL; /* 8 bits */
    settings.c_lflag &= ~ICANON; /* canonical mode */
    settings.c_oflag &= ~OPOST; /* raw output */
    settings.c_lflag &= ~ECHO; /* no echo of course */

    /* Disable further control modes */
    settings.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    settings.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    settings.c_cc[VMIN] = 0;
    settings.c_cc[VTIME] = 0;

    tcsetattr( inputFileFd, TCSANOW, &settings); /* apply the settings */
    tcflush( inputFileFd, TCOFLUSH);

    return;
}


void UsbInterfaceHandler::slotConfChangeAnnounce( const Glib::ustring &parameterId, const Glib::ustring &value, int &handlerMask )
{
    (void)value;
    if( (parameterId == "ipcSerialBaudrate" ) || (parameterId=="all") )
    {
        handlerMask |= HANDLER_MASK_USB_SERIAL;
    }
}


void UsbInterfaceHandler::slotConfChanged( const int handlerMask )
{
    if( handlerMask & HANDLER_MASK_USB_SERIAL )
    {
        lDebug("### slotConfChanged()\n");
        setPortConfiguration();
    }
}


bool UsbInterfaceHandler::OnIOCallback(Glib::IOCondition condition)
{
    bool sta=false;
    Glib::IOStatus status;
    gsize readSize=0;

    if ((condition & Glib::IO_IN) == 0) {
      PRINT_DEBUG ("UsbInterfaceHandler::OnIOCallback() Invalid fifo response");
      return(false);
    }

    try {
	status=_iochannel->read(readBuffer, readBufferSize, readSize);
    } catch (std::exception & e) {
	PRINT_ERROR ("iochannel read yields an exception: " << e.what ());
	return false;
    } catch (Glib::Exception & e) {
	PRINT_ERROR ("iochannel read yields Glib::Exception: " << e.what ());
	return false;
    }

    lDebug ("UsbInterfaceHandler::OnIOCallback() status=%d readSize=%d\n", status, readSize);
    switch (status)
    {
        case Glib::IO_STATUS_NORMAL:
            handleInputData(readBuffer, readSize);
            sta=TRUE;
            break;
        case Glib::IO_STATUS_ERROR:
            discardReceiveFrame();
            sta=false;
            break;
        case Glib::IO_STATUS_EOF:
	    /* upon EOF we disconnect the handler
	     */
            sta=false;
            break;
        case Glib::IO_STATUS_AGAIN:
            PRINT_ERROR ("Again");
            sta=TRUE;
            break;
        default:
            PRINT_ERROR ("???");
            g_return_val_if_reached (FALSE);
            break;
    }

    if (condition & Glib::IO_ERR) {
	PRINT_ERROR ("UsbInterfaceHandler::OnIOCallback() error condition on iochannel, shutting it down");
	connection_errored.emit ();
	return false;
    }

    if (condition & Glib::IO_HUP) {
	PRINT_ERROR ("UsbInterfaceHandler::OnIOCallback() hup condition on iochannel, shutting it down");
	return false;
    }


  return(sta);
};


void UsbInterfaceHandler::discardReceiveFrame()
{
    frameData.clear();
}

unsigned int UsbInterfaceHandler::get_current_frame_size ()
{
    if (frameData.size() < sizeof(gint32)) {
	/* we dont have a full integer yet,
	 * just return 0
	 */
	return 0;
    }

    unsigned int ret = ntohl (* (gint32 *)(frameData.c_str ()));

    /* we return frameSize including the size prefix.
     */
    return ret + sizeof (gint32);
}

void UsbInterfaceHandler::handleInputData(gchar *data, gsize size)
{
    lDebug ("Got data: 0x%02X bytes; [0x%02X|'%c'].\n", (int)size, data[0], data[0]);

    for(int i=0; i<(int)size; i++) {
	/* first, we push one char into frameData
	 */
	frameData.push_back(data[i]);

	/* then we check for complete frame
	 */

	unsigned int frame_size = get_current_frame_size ();

	if (frame_size == 0)
	    continue;

	if (frameData.size () == frame_size) {
	    /* frame is complete: emit it (without the size)
	     */
	    lDebug ("frameData complete; frame size=%u bytes\n", frame_size);
	    std::string actual_data (frameData.substr (sizeof (gint32)));
	    lDebug ("sub = \"%s\"\n", actual_data.c_str ());
	    Glib::ustring uactual_data (actual_data);
	    lDebug ("conversion done\n");

	    emitMessage (uactual_data);
	    /* then clear current frame
	     */
	    frameData.clear ();
	}
    }
    lDebug ("exiting from UsbInterfaceHandler::handleInputData\n");
};


void UsbInterfaceHandler::emit_result( const Glib::ustring &result, int tid ) /* virtual */
{
    if (!_online) {
	PRINT_DEBUG ("NOT online.. tried to send:");
	PRINT_DEBUG (result);
	return;
    }

    lDebug("### emit result tid %d: %s\n", tid, result.c_str() );
    transmitMessages.push(
        std::make_pair(ustring::compose("<task tid=\"%1\">%2</task>", tid, result),
                       tid));
    try {
	sendLoop();
    } catch (Glib::Exception & e) {
	PRINT_ERROR ("UsbInterfaceHandler::emit_result(): sendLoop throws an exception: " << e.what ());
    }
}



/// \brief  straightly send a frame to the serial output
///
///         Does not handle any message fragmentation, counters, frame numbers.
///         It does the decimal/Ascii conversion and checksum generation.
void UsbInterfaceHandler::sendFrame( const Glib::ustring *str )
{
    gsize bytes_to_write = str->bytes();
    guint32 size=htonl( bytes_to_write );

    const char * send_str = str->c_str();
    gsize bytes_written;

    lDebug("Sending frame via usb interface\n");

    lHighDebug("Frame SND: payload_size=%d\n", bytes_to_write );

    _iochannel->write((const char *)&size, sizeof(size), bytes_written);
    lHighDebug("Frame SND: wrote size. bytes_written=%d\n", (int)bytes_written );


    while (bytes_to_write) {
	Glib::IOStatus status = _iochannel->write (send_str, bytes_to_write, bytes_written);
	lHighDebug("Frame SND: tried to write %d bytes, bytes_written=%d\n", (int) bytes_to_write, (int)bytes_written );

	send_str += bytes_written;
	bytes_to_write -= bytes_written;
	if (status == Glib::IO_STATUS_ERROR) {
	    lError ("Frame SND: error during write\n");
	    break;
	}
	if (status == Glib::IO_STATUS_AGAIN) {
	    _iochannel->flush();
	}
    }

    if( doInternLog( ELogLevelHighDebug ) )
        hexdump_mem( str->c_str(), str->size() );

    _iochannel->flush();
    tcdrain (inputFileFd);

    return;
}


/// \brief  check if there is something to send
gboolean UsbInterfaceHandler::sendLoop()
{
    lHighDebug("Send loop\n");

    if(transmitMessages.size())
    {
	auto&& msg = transmitMessages.front();
	lHighDebug("Starting transmitting a new message\n");

	sendFrame( &msg.first );
        if (is_sic_request(msg.second))
            request_sent.emit(msg.second);
	transmitMessages.pop();
    }

    return false;
}


void UsbInterfaceHandler::emitMessage (const Glib::ustring &payload)
{
    std::stringstream ss;
    xmlpp::Element *element;
    xmlpp::DomParser parser;
    int tid;

    // Unfortunately we have to know if the message is an "request" or an
    // "respone" because they are handeled in different channels.

    try {
	parser.parse_memory( payload );
    } catch (std::exception & e) {
	PRINT_ERROR ("unable to parse message: " << e.what ());
	return;

    }
    element=parser.get_document()->get_root_node();
    if (!element) {
	PRINT_ERROR ("no element at rootnode, aborting");
	return;
    }
    assert(element);

    if( element->get_name() != "task" ) {
	PRINT_ERROR ("Invalid message received, ignoring it");
        return;
    }

    ss << element->get_attribute_value("tid");
    ss >> tid;

    element=dynamic_cast<xmlpp::Element *> ( element->get_first_child() );

    if(!element)
    {
        lError( "Could not get child for task\n" );
        return;
    }

    if( element->get_name() == "function" ) {
        UStringIStream is_buf ( element_to_string(element, 0, 0) );
        std::istream is(&is_buf);
        lDebug("Getting function: %s\n", element_to_string(element, 0, 0).c_str() );
        request_ready.emit ( is, tid );
    } else if( element->get_name() == "reply" ) {
	PRINT_DEBUG ("Got a reply for tid " << tid);
	try {
	    response_ready.emit (element_to_string (element, 0, 0), tid);
	    PRINT_DEBUG ("response emitted " << tid);
	} catch (std::exception & e) {
	    PRINT_DEBUG ("exception here " << e.what ());

	    Glib::RefPtr <XmlResult> err_result=XmlResultInternalDeviceError::create (e.what ());
	    response_ready.emit (err_result->to_xml (), tid);
	}
    } else {
        lError("Unknown Message received: %s", element->get_name().c_str() );
    }

    return;
}


/// \brief  Actually send a tests message
///
///         Just dummy xml. Reasonable to test this class e.g. with application
///         runnig twice on differen uart ports connected to each other.
void UsbInterfaceHandler::sendTestMessage()
{
    const Glib::RefPtr<Glib::IOChannel> channel =
    //        Glib::IOChannel::create_from_file("testxmls/get_env_named.xml", "r");
    //        Glib::IOChannel::create_from_file("testxmls/get_env_all.xml", "r");
              Glib::IOChannel::create_from_file("testxmls/get_env_given.xml", "r");
    //        Glib::IOChannel::create_from_file("testxmls/get_env_zix.xml", "r");
    //        Glib::IOChannel::create_from_file("testxmls/set_env_node.xml", "r");
    //        Glib::IOChannel::create_from_file("testxmls/set_env_given.xml", "r");
    //        Glib::IOChannel::create_from_file("testxmls/set_env_given_multiple.xml", "r");
    //        Glib::IOChannel::create_from_file("testxmls/set_env_erroneous_1.xml", "r");
    Glib::ustring str;
    Glib::IOStatus status;

    status=channel->read_to_end(str);
    if(status!=Glib::IO_STATUS_NORMAL)
    {
        lError("Error: Could not read file; %d\n", (int)status);
        return;
    }

    sendMessage( str, 0 );

    return;
};


/// \brief  Actually send a tests message
///
///         Just dummy xml. Reasonable to test this class e.g. with application
///         runnig twice on differen uart ports connected to each other.
void UsbInterfaceHandler::sendMessage (const Glib::ustring &str, int tid)
{
    PRINT_DEBUG ("UsbInterfaceHandler::sendMessage()");

    /* check, whether we are currently online
     */
    if (!_online) {
	PRINT_DEBUG ("NOT online.. tried to send:");
	PRINT_DEBUG (str.raw ());

	throw UsbInterfaceOffline ();
    }

    /* ok, we are online, do it
     */
    transmitMessages.push (std::make_pair(str, tid));

    try {
	PRINT_DEBUG ("### Going to send message:" << str.raw ());
    } catch (Glib::Exception & e) {
	PRINT_ERROR ("UsbInterfaceHandler::sendMessage(): unable to convert message " << e.what ());
    }

    try {
	sendLoop();
    } catch (Glib::Exception & e) {
	PRINT_ERROR ("UsbInterfaceHandler::sendMessage(): sendLoop throws an exception: " << e.what ());
	throw UsbInterfaceOffline ();
    }

    return;
};

void
UsbInterfaceHandler::cancel ()
{ }

void
UsbInterfaceHandler::reset_connection ()
{
    lDebug ("UsbInterfaceHandler::reset_connection ()\n");

    /* close io channel
     */
    _iochannel->close ();

    /* clear all pending tx messages
     */
    PRINT_DEBUG ("Clearing TX Queue:");
    while (!transmitMessages.empty ()) {
	PRINT_DEBUG ("TX queue content: " << transmitMessages.front ().first);
	transmitMessages.pop ();
    }

    /* block message emission until we are online
     * again.
     */
    _online = false;

    /* now we wait
     */
    sigc::slot<bool> tslot=sigc::mem_fun(*this, &UsbInterfaceHandler::on_reconnect_timeout);
    _reset_timeout_connection=Glib::signal_timeout().connect(tslot, USBSERIAL_RECONNECT_TIMEOUT);
}

bool UsbInterfaceHandler::on_reconnect_timeout ()
{
    lDebug ("UsbInterfaceHandler::on_reconnect_timeout ()\n");
    inputFileFd=open(_device_name.c_str(), O_RDWR | O_NONBLOCK);
    if(inputFileFd<=0) {
	/* interface is still not there
	 * return true, so that the timeout is not
	 * disconnected
	 */
	lDebug ("UsbInterfaceHandler::on_reconnect_timeout: could not open fd, restart timeout\n");
	return true;
    }

    lDebug ("UsbInterfaceHandler::on_reconnect_timeout: file \"%s\" opened successfully\n", _device_name.c_str());
    setPortConfiguration();

    _iochannel = Glib::IOChannel::create_from_fd( inputFileFd );
    _iochannel->set_encoding("");
    _iochannel->set_buffered(false);

    // Reset the whole state machine
    discardReceiveFrame();

    /* mark ourselves as online again
     */
    _online = true;

    Glib::signal_io().connect(sigc::mem_fun(*this, &UsbInterfaceHandler::OnIOCallback),
                              _iochannel, Glib::IO_IN );

    resetted.emit ();

    /* returning false from a timeout handler
     * means, that its disconnected.
     *
     * Thats what we want now.
     */
    return false;
}

//---fin.---------------------------------------------------------------------
