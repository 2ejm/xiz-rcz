#ifndef USB_INTERFACE_HANDLER_H
#define USB_INTERFACE_HANDLER_H
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
#include "serial_interface_handler.h"
#include "serial_message.h"
#include "zix_interface.h"


//---Defines-------------------------------------------------------------------


#define USBSERIAL_BUFFER_SIZE              ( 0x1000 * 2 )


//---Declaration---------------------------------------------------------------

class UsbInterfaceOffline : public std::exception
{
    public:
	UsbInterfaceOffline ();
	~UsbInterfaceOffline ();

	virtual const char *what () const noexcept;
};

class UsbInterfaceHandler:   public InterfaceHandler
                            ,public SerialInterface
{
    private:
        int inputFileFd;
        int readBufferSize;
        gchar *readBuffer;
	std::string _device_name;

        Glib::RefPtr<Glib::IOChannel> _iochannel;

        ESerialHandlerMode eSerialHandlerMode;

        // We use std::string instead of Glib::ustring to avoid problems with
        // UTF-8 when handling incomplete UTF-Sequences
        std::string frameData;


        // receiveMessages are not necessary at the moment; we directly emit an
        // message when it arrives
        // std::queue < const Glib::ustring > receiveMessages;
        std::queue < std::pair< Glib::ustring, int > > transmitMessages;

        //Glib::RefPtr<CSerialMessage> senderMessage;
        //Glib::RefPtr<CSerialMessage> receiverMessage;

        int dcTaskId;
        int sicTaskId;

        sigc::connection _reset_timeout_connection;

	unsigned int get_current_frame_size ();

	bool _online;

    protected:
        void setPortConfiguration();

    public:
        UsbInterfaceHandler(ZixInterface inf, const char *deviceName, int deviceSpeed, Glib::RefPtr <XmlProcessor> xml_processor, ESerialHandlerMode _eSerialHandlerMode );
        ~UsbInterfaceHandler();

        static Glib::RefPtr <UsbInterfaceHandler>
            create (ZixInterface inf, const char *deviceName, int deviceSpeed, Glib::RefPtr <XmlProcessor> xml_processor, ESerialHandlerMode _eSerialHandlerMode);

        void slotConfChangeAnnounce( const Glib::ustring &resource, const Glib::ustring &value, int &handlerMask);
        void slotConfChanged( const int handlerMask );

        void handleInputData(gchar *input, gsize size);
        virtual void emit_result (const Glib::ustring &result, int tid);
        //void discardReceiveFrame();
        void sendFrame( const Glib::ustring *str );
        int digestFrame();
        void discardReceiveFrame();

        gboolean sendPayloadFrameFromMessage();
        gboolean sendLoop();

        void sendTestMessage();
        void sendMessage( const Glib::ustring &str, int tid );

        void emitMessage(const Glib::ustring &payload);

        bool OnIOCallback(Glib::IOCondition condition);
        void cancel ();

	bool on_reconnect_timeout ();
	virtual void reset_connection ();
};


//-----------------------------------------------------------------------------
#endif // ? ! USB_INTERFACE_HANDLER_H
