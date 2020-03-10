//-----------------------------------------------------------------------------
///
/// \brief  Serial interface handler
///
///         Handles fragmentation of messages into frames.
///
///         Messages get split before sending and received frames get merged to
///         messages again.
///         Each frame of a message has to be acknowledged by the counterpart.
///
///         How this class tries to manage it:
///
///         There are two state machines for Mesages (not frames!);
///
///             * Messages from SIC to devicecontroller; "Sender"
///             * Messages from devicecontroller to SIC; "Receiver"
///
///         The state machines are handling messages, not actually sending or
///         receving frames. So the Sending state machine might be also waiting
///         for a (confirmation-) frame to receive for example
///
///         There is one handler for receiving frames (Must not send data!) and
///         one handler that sends frames.
///
///         Both handler control both state machines. This guarantees that
///         full duplex messaging communication does work and single send frames
///         don't get mixed up.
///
///                                 |
///                           Input-Handler
///                            /          |
///                 Sender-State-M.     Receiver-State-M.
///                            \         /
///                           Output-Handler
///                                 |
///
/// \todo   Use signals! Don't call send-handler manually!
///
/// \date   [20161114] File created
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
#include <glibmm/convert.h>            // signal_io
#include <fcntl.h>                  // O_RDWR
#include <iomanip>                  // std::setfill
#include <termios.h>                // speed_t, tcgetattr
#include <cassert>
#include <memory>

//---Own------------------------------

#include "serial_interface_handler.h"
#include "crc32.h"
#include "ustring_istream.h"
#include "conf_handler.h"           // Setting for serial port
#include "hexio.h"
#include "log.h"
#include "xml_helpers.h"
#include "procedure_step_handler.h"
#include "xml_result_timeout.h"
#include "time_utilities.h"
#include "serial_helper.h"

//---Forward declarations------------------------------------------------------


//---Implementation------------------------------------------------------------


using Glib::ustring;

SerialInterfaceHandler::SerialInterfaceHandler(ZixInterface inf, const char *deviceName, int deviceSpeed, Glib::RefPtr <XmlProcessor> xml_processor, ESerialHandlerMode _eSerialHandlerMode)
    : Glib::ObjectBase ("SerialInterfaceHandler")
    , InterfaceHandler (inf, inf.to_string(), xml_processor)
    , SerialInterface()
    , _current_ack_i (1)
    , _current_ack_n (1)
    , _current_mid (0)
    , _received_mid (0)
    , _need_ack_mid (-1)
    , eSerialHandlerMode(_eSerialHandlerMode)
    // ,signalTimeout(Glib::MainContext::get_default())
{
    Glib::RefPtr <ConfHandler> conf=ConfHandler::get_instance();

    std::string strDeviceName=deviceName;
    std::string strFileMode="a+";
    Glib::ustring unit, value;
    int serialMaxPlayloadSize=0;

    (void)deviceSpeed;

    // Reset the state machine
    discardReceiveFrame();

    eSerialSenderState=eSerialSenderStateIdle;
    eSerialReceiverState=eSerialReceiverStateIdle;

    dcTaskId=0;
    _injectWrongCrc=0;
    doIgnoreWrongCrc=false;
    doCrcSkipHeader=false;

    if(conf->getConf("numberOfBytesPerFrameMax", unit, value))
    {
        std::stringstream s;
        s << value;
        s >> serialMaxPlayloadSize;
    }

    if(!serialMaxPlayloadSize)
        serialMaxPlayloadSize=SERIAL_MAX_PAYLOAD_SIZE;

    senderMessage=Glib::RefPtr<CSerialMessage> ( new CSerialMessage( serialMaxPlayloadSize ) );
    receiverMessage=Glib::RefPtr<CSerialMessage> ( new CSerialMessage( serialMaxPlayloadSize ) );

    inputFileFd=open(deviceName, O_RDWR | O_NONBLOCK);
    if(inputFileFd<=0)
        throw("Could not open file");

    setPortConfiguration();

    _iochannel = Glib::IOChannel::create_from_fd( inputFileFd );
    _iochannel->set_encoding("");
    _iochannel->set_buffered(false);

    readBufferSize=SERIAL_BUFFER_SIZE;
    readBuffer=new gchar[readBufferSize];

    register_connection (std::shared_ptr <InterfaceConnection> (this) );

    Glib::signal_io().connect(sigc::mem_fun(*this, &SerialInterfaceHandler::OnIOCallback),
                              _iochannel, Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP);

    conf->confChangeAnnounce.connect(
        sigc::mem_fun(*this, &SerialInterfaceHandler::slotConfChangeAnnounce ));
    conf->confChanged.connect(
        sigc::mem_fun(*this, &SerialInterfaceHandler::slotConfChanged ));

    //setLogLevel(ELogLevelHighDebug);
};


Glib::RefPtr <SerialInterfaceHandler>
SerialInterfaceHandler::create (ZixInterface inf, const char *deviceName, int deviceSpeed, Glib::RefPtr <XmlProcessor> xml_processor, ESerialHandlerMode _eSerialHandlerMode) /* static */
{
    return Glib::RefPtr <SerialInterfaceHandler> (new SerialInterfaceHandler (inf, deviceName, deviceSpeed, xml_processor, _eSerialHandlerMode) );
};


SerialInterfaceHandler::~SerialInterfaceHandler()
{
    delete[] readBuffer;
    readBuffer=NULL;

    return;
};


void SerialInterfaceHandler::setPortConfiguration()
{
    Glib::RefPtr <ConfHandler> conf=ConfHandler::get_instance();
    Glib::ustring unit, value;

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

    settings.c_cc[VMIN] = 1;
    settings.c_cc[VTIME] = 0;

    tcsetattr( inputFileFd, TCSANOW, &settings); /* apply the settings */
    tcflush( inputFileFd, TCOFLUSH);

    return;
}
void SerialInterfaceHandler::midHandler()
{
    _current_mid = _current_mid + 1;
    lDebug("midHandler _current_mid:%d",_current_mid);

    if (_current_mid == 1000)
    {
        _current_mid = 0;
    }
}

void SerialInterfaceHandler::slotConfChangeAnnounce( const Glib::ustring &parameterId, const Glib::ustring &value, int &handlerMask )
{
    (void)value;
    lDebug("SerialInterfaceHandler: slotConfChangeAnnounce()\n");

    // We dont reconfigure ipc interfaces at any time at the moment.
    // parameterId "all" also should not work
    if( (parameterId == "ipcSerialBaudrate" )
         || ( parameterId == "all" ) )
    {
        handlerMask |= HANDLER_MASK_IPC_SERIAL;
    }
}


void SerialInterfaceHandler::slotConfChanged( const int handlerMask )
{
    if( handlerMask & HANDLER_MASK_IPC_SERIAL )
    {
        lDebug("Updating configuratio0n for ipc serial");
        setPortConfiguration();
    }
}


bool SerialInterfaceHandler::OnIOCallback(Glib::IOCondition condition)
{
    bool sta=false;
    //gchar buf[0x20];    // If ( size != 1 ) its blocking until bytes are read. Not very efficient
    Glib::IOStatus status;
    gsize readSize=0;

    PRINT_DEBUG ("SerialInterfaceHandler::OnIOCallback() entered");

    if ((condition & Glib::IO_IN) == 0) {
      PRINT_DEBUG ("SerialInterfaceHandler::OnIOCallback() Invalid fifo response");
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

    lDebug ("SerialInterfaceHandler::OnIOCallback() status=%d readSize=%d\n", status, readSize);
    switch (status)
    {
        case Glib::IO_STATUS_NORMAL:
            //g_warning ("Handle");
            handleInputData(readBuffer, readSize);
            sta=TRUE;
            break;
        case Glib::IO_STATUS_ERROR:
            PRINT_ERROR ("IO error\n");
            discardReceiveFrame();
            sta=FALSE;
            break;
        case Glib::IO_STATUS_EOF:
            PRINT_DEBUG ("No input data available");
            sta=TRUE;
            break;
        case Glib::IO_STATUS_AGAIN:
            PRINT_ERROR ("Again");
            sta=TRUE;
            break;
        default:
            g_warning ("???");
            g_return_val_if_reached (FALSE);
            break;
    }

  return(sta);
};


gboolean SerialInterfaceHandler::handleInputData(gchar *data, gsize size)
{
    lDebug ("Got data: 0x%02X bytes; [0x%02X|'%c'].\n", (int)size, data[0], data[0]);

    for(int i1=0; i1<(int)size; i1++)
    {
        switch(eSerialReceiveFrameState)
        {
            case eSerialReceiveFrameStateWaitForStart:
                if( data[i1] == eSerialCodeStart )
                {
                    //printf ("Starting Frame\n");
                    eSerialReceiveFrameState=eSerialReceiveFrameStateReadHeader;
                    frameData.clear();
                    frameData.push_back(data[i1]);
                    pos=1;
                    FTO_timer_start();
                }
                else
                {
                    lError ("Unknown byte out of frame stream: 0x%02X\n", data[i1]);

		    /* no associated Frame, we set 1/1 */
		    _current_ack_i = 1;
		    _current_ack_n = 1;
                    sendConfirmFrame( eSerialCodeNAK, "NF");
                }
                break;
            case eSerialReceiveFrameStateReadHeader:
                //serialHeader.data[pos++]=data[i1];
                pos++;
                frameData.push_back(data[i1]);
                if(pos>=(int)sizeof(SSerialHeader))
                {
                    eSerialReceiveFrameState=eSerialReceiveFrameStateReadPayload;
                    pos=0;
                }
                break;
            case eSerialReceiveFrameStateReadPayload:
                //frameData.push_back(data[i1]);
                frameData.push_back(data[i1]);
                pos++;
                if( data[i1] == eSerialCodeStop )
                {
                    //printf("Frame finished\n");
                    digestFrame();
                    eSerialReceiveFrameState=eSerialReceiveFrameStateWaitForStart;
                    pos=0;
                    lDebug("FTO Timer Stop!");
                    FTO_timerConnection.disconnect();
                }
                break;
            default:
                FTO_timerConnection.disconnect();
                lError ("Unknown serial state: 0x02%X\n", eSerialReceiveFrameState);
                discardReceiveFrame();
                break;
        }
    }

    lDebug( "eSerialState=0x%02X; pos=0x%02X\n", eSerialReceiveFrameState, pos);

    return TRUE;
};


void SerialInterfaceHandler::emit_result( const Glib::ustring &result, int tid) /* virtual */
{
    lDebug("### emit result: %s\n", result.c_str() );
    transmitMessages.push(
        std::make_pair(ustring::compose("<task tid=\"%1\">%2</task>", tid, result),
                       tid));
    sendLoop();
}


/// \brief  reset state machine
void SerialInterfaceHandler::discardReceiveFrame()
{
    pos=0;
    eSerialReceiveFrameState=eSerialReceiveFrameStateWaitForStart;
    return;
}


/// \brief  straightly send a confirmation frame to the serial output
///
///         Just calls sendFrame()
void SerialInterfaceHandler::sendConfirmFrame( ESerialCode ack, const char *code)
{
    /*
    SSerialHeader header;
    header.start=eSerialCodeStart;
    header.type=eSerialCodeStart;
    char data[2];
    data[0]=ack;
    data[1]=code;
    fwrite(&header, sizeof(header), 1, inputFile);
    guint32 c=0;
    //c=crc32(c, (const char *)&header, sizeof(header) );
    */
    std::string str;
    str.push_back((char)ack);
    if( ack != eSerialCodeACK )
        str+=code;

    sendFrame( _received_mid, eSerialCodeConfirmation, _current_ack_i, _current_ack_n, &str);
    lDebug("Sending ConfirmFrame with mid:%d",_received_mid);
}


/// \brief  straightly send a payload frame to the serial output
///
///         Just calls sendFrame()
void SerialInterfaceHandler::sendPayloadFrame(int i, int n, const std::string *str)
{
    midHandler();
    _need_ack_mid = _current_mid;
    sendFrame(_current_mid, eSerialCodePayload, i, n, str);
    return;
}

void SerialInterfaceHandler::error_in_confirmation()
{
    if (_error_counter < _error_max)
	{
        _error_counter++;
        lDebug("Resending %d th time", _error_counter);
        resendPayloadFrame();

	}
	else
	{
        lDebug("Giving up error re-Sending frame.");
        senderMessage->discard();
        eSerialSenderState=eSerialSenderStateIdle;
        _error_counter=0;
	}

}
void SerialInterfaceHandler::reset_error_counters()
{
    _error_counter=0;
    _timeout_counter=0;
}

void SerialInterfaceHandler::resendPayloadFrame()
{
    gsize bytes_written;
    lDebug("RESEND");
    lDebug("Resending Frame with mid:%d,_need_ack_mid:%d,  _current_mid:%d", _sent_frame_mid, _need_ack_mid,_current_mid);

    _iochannel->write((const char *) _sent_frame.data(), _sent_frame.bytes(), bytes_written);
    _iochannel->flush();
    tcdrain(inputFileFd);

    eSerialSenderState=eSerialSenderStateWaitForAck;
	// ensure
    _need_ack_mid = _sent_frame_mid;
    confirmation_timer_start();
}
/// \brief  straightly send a frame to the serial output
///
///         Does not handle any message fragmentation, counters, frame numbers.
///         It does the decimal/Ascii conversion and checksum generation.
void SerialInterfaceHandler::sendFrame( int mid, int type, int i, int n, const std::string *str)
{
    Glib::ustring header;
    Glib::ustring footer;
    int crc32_value=0;
    //char data[8+1];
    gsize bytes_written;

    header=ustring::compose("%1%2%3%4%5"
                , (char)eSerialCodeStart
                , (char)type
                , ustring::format(std::dec, std::setfill(L'0'), std::setw(4), mid)
                , ustring::format(std::dec, std::setfill(L'0'), std::setw(4), i)
                , ustring::format(std::dec, std::setfill(L'0'), std::setw(4), n)
            );


    lDebug("Straightly sending frame\n");
    //fwrite(&header, 1, sizeof(header), outputFile);
    _iochannel->write((const char *)header.data(), header.bytes(), bytes_written);
    //fwrite(str->c_str(), 1, str->size(), outputFile);
    _iochannel->write(*str);

    if(!doCrcSkipHeader)
        crc32_value=crc32(0, (const char *)header.data(), header.bytes());

    crc32_value=crc32(crc32_value, str->c_str(), str->size());

    if(_injectWrongCrc)
    {
        lWarn("Injecting wrong crc (%d)\n", _injectWrongCrc);
        crc32_value=0x12345678;
        _injectWrongCrc=0;
    }

    footer=ustring::compose("%1%2"
                , ustring::format(std::hex, std::setfill(L'0'), std::uppercase, std::setw(8), crc32_value)
                , (char)eSerialCodeStop
                );

    _iochannel->write((const char *)footer.data(), footer.bytes(), bytes_written);

    if( type == eSerialCodePayload )
    {
        if( !str->size() )
        {
            lError("### Error: SND: empty payload frame\n");
            throw("Could not open file");
        }
    }

    lDebug("Frame SND: Type=%c; mid=%d, i=%d; n=%d; CRC=0x%08X; payload_size=%d; footer_size=%d; bytes_written=%d\n", type, mid, i, n, crc32_value, (int)str->size(), (int)footer.bytes(), (int)bytes_written );
    lHighDebug("   ");
    if( doInternLog( ELogLevelHighDebug ) )
        hexdump_mem( str->c_str(), str->size() );

    _iochannel->flush();
    tcdrain(inputFileFd);

    if (type == eSerialCodePayload) {
	_sent_frame = ustring::compose("%1%2%3", header, *str, footer);
	_sent_frame_mid = mid;
	confirmation_timer_start();
    }
    return;
}


/// \brief  parse frame and fill data into message
///
///         Call this method when a frame was received. It will check how to
///         manage it
int SerialInterfaceHandler::digestFrame()
{
    lHighDebug("Digest frame\n");
    uint32_t    crc32_value, crc32_should=0;
    int payload_size=0;
    int i, n, mid;
    char data[8+1];
    std::stringstream ss;

    SSerialHeader *header=(SSerialHeader *)frameData.data();
    SSerialFooter *footer=(SSerialFooter *)&frameData.data()[ frameData.size() - sizeof(*footer)];

    // CRC32 of Frame
    ss.clear();
    memcpy(data, footer->crc32, sizeof(footer->crc32) );
    data[ sizeof(footer->crc32) ]=0;
    ss << std::hex << data;
    ss >> crc32_value;

    // Index of Mid
    ss.clear();
    memcpy(data, header->asciiMid, sizeof(header->asciiMid) );
    data[ sizeof(header->asciiMid) ]=0;
    ss << std::dec << data;
    ss >> mid;
    // Index of Frame
    ss.clear();
    memcpy(data, header->asciiIndex, sizeof(header->asciiIndex) );
    data[ sizeof(header->asciiIndex) ]=0;
    ss << std::dec << data;
    ss >> i;

    // Number of Frames
    ss.clear();
    memcpy(data, header->asciiNumber, sizeof(header->asciiNumber) );
    data[ sizeof(header->asciiNumber) ]=0;
    ss << std::dec << data;
    ss >> n;

    payload_size=frameData.size() - ( sizeof( *header ) + sizeof( *footer ));

    #if 0
        printf("DATA: ");
        for(int i1=0; i1<frameData.size(); i1++)
            printf("[%02X|%c] ", frameData.data()[i1], frameData.data()[i1]);
        printf("\n");
    #endif
    _received_mid = mid;

    lDebug("Frame RCV: Type=%c;mid=%d; i=%d; n=%d; CRC=0x%08X; payload_size=%d; total_size=%d\n", header->type, mid, i, n, crc32_value, payload_size, (int)frameData.size());
    if(!doCrcSkipHeader)
        crc32_should=crc32(crc32_should, (const char *)header, sizeof( *header ) + payload_size);
    else
        crc32_should=crc32(crc32_should, (const char *)frameData.data() + sizeof( *header ), payload_size);
    lDebug("   CRC:%s (0x%08X/0x%08X)\n",  (crc32_should == crc32_value)?"OK":"Failed",
           crc32_value, crc32_should
           );

    if( header->type==eSerialCodePayload )
    {
        if( ( ! doIgnoreWrongCrc ) && (crc32_should != crc32_value) )
        {
	    _current_ack_i = i;
	    _current_ack_n = n;
            eSerialReceiverState=eSerialReceiverStateWrongCrc;
            sendLoop();
        }
        else
        {
            // First frame? initialize receverMessage
            if(i==1)
                receiverMessage->initReceiverMessage(n);

	    std::string framePayload=frameData.substr(sizeof( *header ), payload_size);
	    lHint("Pushing data to frame: [%d/%d]\n", i, n ); // , framePayload.c_str()

            receiverMessage->pushFrame(i,n,framePayload );
	    _current_ack_i = i;
	    _current_ack_n = n;

            eSerialReceiverState=eSerialReceiverStateGotPayload;
            if( !payload_size )
            {
                lFatal("### Error: RCV empty payload frame\n");
                throw("Error: RCV empty payload frame");
            }
            sendLoop();
        }
    }
    else
    {
        if(crc32_should != crc32_value)
        {
            // Confirmation has wrong CRC but i cant handle that.
            lError("Error: Confirmation has wrong CRC. I have to ignore that, no other chance.\n");
        }
        else

        {
            if( eSerialSenderState != eSerialSenderStateWaitForAck )
            {
                lError("Error: Transceiver was not waiting for Ack but got one\n");
                eSerialSenderState = eSerialSenderStateIdle;
            }
            else
            {
                Glib::ustring framePayload=frameData.substr(sizeof( *header ), payload_size);
                if( framePayload[0] != eSerialCodeACK )
                {
                    lError("Error: Got NAK instead of ACK; got %s still ignoring it.\n", framePayload.c_str() );
                    eSerialSenderState = eSerialSenderStateGotNak;
                }
                else
                {
                    eSerialSenderState = eSerialSenderStateGotAck;

                }
            }
            // TBD To be done: just emmit signal that send loop gets called.
            // Dont ever call this directly!
            sendLoop();
        }
    }
    return(0);
}


/// \brief  Pops frame from curretn sending message and send it
gboolean SerialInterfaceHandler::sendPayloadFrameFromMessage()
{
    senderMessage->popFrame();
    eSerialSenderState=eSerialSenderStateWaitForAck;
    sendPayloadFrame( senderMessage->getTransceived(),
                      senderMessage->numberOfFrames(),
                      &senderMessage->getFramePayload() );

    return(false);
}


/// \brief  check if there is something to send
gboolean SerialInterfaceHandler::sendLoop()
{
    lDebug("Send loop\n");

    // Send message statemachine
    switch( eSerialSenderState )
    {
        case eSerialSenderStateIdle:
            if(transmitMessages.size())
            {
                lDebug("Starting transmitting a new message\n");

                senderMessage->initSenderMessage(
                    transmitMessages.front().first, transmitMessages.front().second );
                transmitMessages.pop();

                sendPayloadFrameFromMessage();
            }
            break;
        case eSerialSenderStateWaitForAck:
            lDebug("Waiting for Ack with mid:%d...\n", _need_ack_mid);
            break;
        case eSerialSenderStateGotAck:
            lDebug("eSerialSenderStateGotAck\n");
            lDebug("Stopping Timer");
            confirmation_timerConnection.disconnect();
            // Check is id is correct
            if (_received_mid != _need_ack_mid )
            {
                lError("Wrong frame mid! Expected:%d,Received:%d", _need_ack_mid, _received_mid);
                error_in_confirmation();
            }
            else
			{
                lDebug("Correct mid, Expected-MID:%d, received-mid:%d; _current_mid:%d",_need_ack_mid, _received_mid, _current_mid);
                reset_error_counters();
                eSerialSenderState=eSerialSenderStateIdle;
                // Check for next frame to send
                if( senderMessage->dataLeft() )
                    {
                        lDebug("sendPayloadFrameFromMessage\n");
                        sendPayloadFrameFromMessage();
                    }
                else
                {
                    lDebug("Message completed!\n");
                    if (is_sic_request(senderMessage->tid()))
                        request_sent.emit(senderMessage->tid());

                    // Check for next message to send
                    if (transmitMessages.size()) {
                        lDebug("Starting transmitting directly a new message\n");

                        senderMessage->initSenderMessage(
                            transmitMessages.front().first,
                            transmitMessages.front().second );
                        transmitMessages.pop();

                        sendPayloadFrameFromMessage();
                    }
                }
            }
            break;
        case eSerialSenderStateGotNak:
            confirmation_timerConnection.disconnect();
            error_in_confirmation();
            break;
        default:
            lError("Error: Unknown sender state: %d\n", eSerialSenderState);
    }

    lDebug("Receive message statemachine\n");
    // Receive message statemachine
    switch( eSerialReceiverState )
    {
        case eSerialReceiverStateIdle:
	    lDebug("eSerialReceiverStateIdle\n");
            break;
        case eSerialReceiverStateGotPayload:
	    lDebug("eSerialReceiverStateGotPayload\n");
            if( receiverMessage->messageFinished() )
            {
		Glib::ustring utf8_payload;
		lDebug("got complete message, convert it\n");
		try {
		    utf8_payload = receiverMessage->getPayload();
		    PRINT_DEBUG ("### RCV Message finished: " << utf8_payload.raw());
		} catch (const Glib::ConvertError &e) {
		    lError("Error in Payload conversion\n");
		    sendConfirmFrame(eSerialCodeNAK, "FE");
		    receiverMessage->discard();
		    eSerialReceiverState=eSerialReceiverStateIdle;
		    break;
		}

		lDebug("went through\n");
                eSerialReceiverState=eSerialReceiverStateIdle;
                switch(eSerialHandlerMode)
                {
                    case ESerialHandlerModeCounterpart2:
                    case ESerialHandlerModeNormal:
                    {
			sendConfirmFrame(eSerialCodeACK, NULL);
			eSerialReceiverState=eSerialReceiverStateIdle;
                        emitMessage(utf8_payload);
                        break;
                    }
                    case ESerialHandlerModeCounterpart:
                        // I got an answer from the daemon via RS232.
                        // Print it, ignore it.
                        // We can quit now because nothing will happen any more.
                        exit(0);
                        break;
                    case ESerialHandlerModeDeviceController:
                        // I got an reqeust from the daemon via RS232.
                        // Send an answer
                        sendMessage("<reply status=\"200\">   </reply>", 0);
                        break;
                    default:
                        lFatalExit("unknown eSerialHandlerMode");
                        break;
                }
            } else {
		lDebug("sendConfirmFrame\n");
		sendConfirmFrame(eSerialCodeACK, NULL);
		eSerialReceiverState=eSerialReceiverStateIdle;
	    }
            break;
        case eSerialReceiverStateWrongCrc:
            sendConfirmFrame(eSerialCodeNAK, "CS");
            receiverMessage->discard();
            eSerialReceiverState=eSerialReceiverStateIdle;
            break;
        default:
            lError("Error: Unknown receiver state: %d\n", eSerialReceiverState);
            break;
    }

    lDebug("Debug:\n");
    lDebug("   Sender   : eSerialSenderState: 0x%02X\n", eSerialSenderState);
    lDebug("   Receiver : eSerialReceiverState: 0x%02X\n", eSerialReceiverState);

    return(FALSE);
}

void SerialInterfaceHandler::confirmation_timer_start()
{
	lDebug("Starting Timer!");
	confirmation_timerConnection.disconnect();
	confirmation_timerConnection = Glib::signal_timeout().connect(sigc::mem_fun(*this, &SerialInterfaceHandler::confirmation_timeout_handler), _timeout_confimation);

}
void SerialInterfaceHandler::FTO_timer_start()
{
	lDebug("FTO TIMER Start!");
	FTO_timerConnection.disconnect();
	FTO_timerConnection = Glib::signal_timeout().connect(sigc::mem_fun(*this, &SerialInterfaceHandler::FTO_timeout_handler), _timeout_FTO );

}
bool SerialInterfaceHandler::confirmation_timeout_handler()
{
	lError("Confirmation frame Timeout");
	confirmation_timerConnection.disconnect();

	_timeout_counter = _timeout_counter + 1;

	lDebug("Timeout Confirmation Frame! Senderstate:%d; Timeout-counter:%d", (int)eSerialSenderState, _timeout_counter);
	if (_timeout_counter < _timeout_max)
	{
		resendPayloadFrame();
	}
	else
	{
		lError("No Confirmation received during  %d timouts-> Throwing Message away.", _timeout_counter);
		_timeout_counter=0;
		senderMessage->discard();
		eSerialSenderState=eSerialSenderStateIdle;
	}
	/* timeout dont call again
	 */
    return false;
}
bool SerialInterfaceHandler::FTO_timeout_handler()
{
	lError("FTO-Timeout");
	FTO_timerConnection.disconnect();
	lDebug("SEND NAK FTO");
	sendConfirmFrame( eSerialCodeNAK, "FTO");
	/* timeout dont call again
	 */
	return false;
}

void SerialInterfaceHandler::emitMessage (const Glib::ustring &payload)
{
    std::stringstream ss;
    xmlpp::Element *element;
    xmlpp::DomParser parser;
    int tid;

    // Unfortunately we have to know if the message is an "request" or an
    // "respone" because they are handeled in different channels.
    try{
        parser.parse_memory( payload );
    }
    catch( ... )
    {
        lError("Error: Could not parse xml code!");
        return;
    }

    element=parser.get_document()->get_root_node();
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

    if( element->get_name() == "function" )
    {
        UStringIStream is_buf ( element_to_string(element, 0, 0) );
        std::istream is(&is_buf);
        dcTaskId=tid;
        lDebug("Getting function: %s\n", element_to_string(element, 0, 0).c_str() );
        request_ready.emit ( is, tid );
    }
    else if( element->get_name() == "reply" )
        response_ready.emit ( element_to_string(element, 0, 0), tid );
    else
        lError("Unknown Message received: %s", element->get_name().c_str() );

    return;
}


/// \brief  Actually send a tests message
///
///         Just dummy xml. Reasonable to test this class e.g. with application
///         runnig twice on differen uart ports connected to each other.
void SerialInterfaceHandler::sendTestMessage( const char *fileName )
{
    const Glib::RefPtr<Glib::IOChannel> channel =
              Glib::IOChannel::create_from_file( fileName, "r");
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
void SerialInterfaceHandler::sendMessage(const Glib::ustring &str, int tid)
{
    try {
        PRINT_DEBUG ("### Going to send message:" << str.raw());
    } catch (Glib::Exception & e) {
        PRINT_ERROR ("RS422InterfaceHandler::sendMessage(): unable to convert message " << e.what ());
    }

    transmitMessages.push (std::make_pair(str, tid));

    sendLoop();

    return;
};




void SerialInterfaceHandler::reset_connection ()
{
    /* nothing todo for normal serial
     */

    resetted.emit ();
}

void
SerialInterfaceHandler::cancel ()
{ }


SerialInterface::SerialInterface()
    :InterfaceConnection (ZIX_PROC_RESULT_FILE)
{
    // Just the interface, nothing to do
    return;
}

//---fin.---------------------------------------------------------------------
