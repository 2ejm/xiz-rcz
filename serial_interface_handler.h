#ifndef SERIAL_INTERFACE_HANDLER_H
#define SERIAL_INTERFACE_HANDLER_H
//-----------------------------------------------------------------------------
///
/// \brief  Serial interface handler
///
///         See implementation for further details
///
/// \date   [20161114] File created
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

#include <stdio.h>
#include <string>
#include <list>
#include <glibmm/ustring.h>
#include <glibmm/object.h>
#include <glibmm/bytes.h>
#include <queue>
#include <utility>
#include <giomm/file.h>

//---Own------------------------------

#include "interface_handler.h"
#include "serial_message.h"
#include "zix_interface.h"


//---Defines-------------------------------------------------------------------


#define SERIAL_MAX_PAYLOAD_SIZE         0x800
#define SERIAL_BUFFER_SIZE              ( 0x1000 * 2 )


//---Declaration---------------------------------------------------------------


enum ESerialReceiveFrameState
{
    eSerialReceiveFrameStateIdle           = 0x0,
    eSerialReceiveFrameStateWaitForStart   = 0x0,  // The same like idle
    eSerialReceiveFrameStateReadHeader     = 0x1,
    eSerialReceiveFrameStateReadPayload    = 0x2,
};


enum ESerialSenderState
{
    eSerialSenderStateIdle           = 0x0,
    eSerialSenderStateWaitForAck     = 0x1,
    eSerialSenderStateGotAck         = 0x2,
    eSerialSenderStateGotNak         = 0x3,
};


enum ESerialReceiverState
{
    eSerialReceiverStateIdle           = 0x0,
    eSerialReceiverStateGotPayload     = 0x1,
    eSerialReceiverStateSendAck        = 0x2,
    eSerialReceiverStateWaitForPayload = 0x2, // Same as send Ack
    eSerialReceiverStateWrongCrc       = 0x3,
};


enum ESerialCode
{
    eSerialCodeStart  = 0x01,
    eSerialCodeStop   = 0x04,

    eSerialCodePayload      = 0x50,  // payload frame, 'P'
    eSerialCodeConfirmation = 0x43,  // confirmation frame, 'C'

    eSerialCodeACK    = 0x06,
    eSerialCodeNAK    = 0x15,

    // TBD: Codes to be clarified
    eSerialCodeCS     = 0x10,
    eSerialCodeNF     = 0x11,
    eSerialCodeFTO    = 0x12,
    eSerialCodeFE     = 0x13,

    eSerialCodeNone   = 0xff,      // internal only
};


struct SSerialHeader
{
    char start;
    char type;
    char asciiMid[4];
    char asciiIndex[4];
    char asciiNumber[4];
} __attribute__ ( ( packed ) );


struct SSerialFooter
{
    char crc32[8];
    char eot;
} __attribute__ ( ( packed ) );


enum ESerialHandlerMode
{
    ESerialHandlerModeNormal                = 0x01,
    ESerialHandlerModeDeviceController      = 0x02,
    ESerialHandlerModeCounterpart           = 0x03,
    ESerialHandlerModeCounterpart2          = 0x04,
};


class SerialInterface: public InterfaceConnection

{
    private:

    protected:

        bool is_sic_request(int tid) const
        {
            // odd: request from SIC
            return tid % 2 != 0;
        }

    public:
        SerialInterface();

	virtual void sendMessage( const Glib::ustring &str, int tid ) = 0;
	virtual void reset_connection () = 0;

	sigc::signal <void>  resetted;
	sigc::signal <void, int> request_sent;
};


class SerialInterfaceHandler: public InterfaceHandler
                             ,public SerialInterface

{
    private:
        int inputFileFd;
        int readBufferSize;
        gchar *readBuffer;

        Glib::RefPtr<Glib::IOChannel> _iochannel;

        ESerialSenderState eSerialSenderState;
        ESerialReceiverState eSerialReceiverState;
	int _current_ack_i;
	int _current_ack_n;

	int _current_mid;
	int _received_mid;
	int _need_ack_mid;


	int _timeout_confimation = 500;
	int _timeout_FTO = 100;

	int _error_counter;
	int _timeout_counter;

	const int _timeout_max = 5;
	const int _error_max = 3;
        ESerialReceiveFrameState eSerialReceiveFrameState;
        ESerialHandlerMode eSerialHandlerMode;

        int pos;
        // We use std::string instead of Glib::ustring to avoid problems with
        // UTF-8 when handling incomplete UTF-Sequences
        std::string frameData;

        // receiveMessages are not necessary at the moment; we directly emit an
        // message when it arrives
        // std::queue < const Glib::ustring > receiveMessages;
        std::queue < std::pair< Glib::ustring, int > > transmitMessages;

        Glib::RefPtr<CSerialMessage> senderMessage;
        Glib::RefPtr<CSerialMessage> receiverMessage;

        int dcTaskId;
        int sicTaskId;
        int _injectWrongCrc;
        bool doIgnoreWrongCrc;
        bool doCrcSkipHeader;

        sigc::connection confirmation_timerConnection;
        sigc::connection FTO_timerConnection;

        Glib::ustring _sent_frame;
        int _sent_frame_mid;
    protected:
        void setPortConfiguration();
        void midHandler();
        void reset_error_counters();

    public:
        void injectWrongCrc()
        {
            _injectWrongCrc=1;
        }
        SerialInterfaceHandler(ZixInterface inf, const char *deviceName, int deviceSpeed, Glib::RefPtr <XmlProcessor> xml_processor, ESerialHandlerMode _eSerialHandlerMode );
        ~SerialInterfaceHandler();

        static Glib::RefPtr <SerialInterfaceHandler>
            create (ZixInterface inf, const char *deviceName, int deviceSpeed, Glib::RefPtr <XmlProcessor> xml_processor, ESerialHandlerMode _eSerialHandlerMode);

        void slotConfChangeAnnounce( const Glib::ustring &resource, const Glib::ustring &value, int &handlerMask);
        void slotConfChanged( const int handlerMask );

        gboolean handleInputData(gchar *input, gsize size);
        virtual void emit_result( const Glib::ustring &result, int tid );
        void discardReceiveFrame();
        void sendPayloadFrame(int i, int n, const std::string *str);
        void sendConfirmFrame( ESerialCode ack, const char *code);
        void sendFrame(int mid, int type, int i, int n, const std::string *str);
        void resendPayloadFrame();
        int digestFrame();

        void error_in_confirmation();
        gboolean sendPayloadFrameFromMessage();
        gboolean sendLoop();

        void sendTestMessage( const char *fileName );
        virtual void sendMessage( const Glib::ustring &str, int tid);

        void emitMessage(const Glib::ustring &payload);

        bool OnIOCallback(Glib::IOCondition condition);
        void cancel ();

        bool on_timeout_once();
        bool confirmation_timeout_handler();
        bool FTO_timeout_handler();
        void confirmation_timer_start();
        void FTO_timer_start();
	virtual void reset_connection ();
};


//-----------------------------------------------------------------------------
#endif // ? ! SERIAL_INTERFACE_HANDLER_H
