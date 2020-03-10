#ifndef SERIAL_MESSAGE_H
#define SERIAL_MESSAGE_H
//-----------------------------------------------------------------------------
///
/// \brief  Serial message
///
///         handles splitting and reassembling messages
///
///         A message can either be a senders message or a receivers message.
///
/// \date   [20161118] File created
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

#include <string>
#include <glibmm/ustring.h>
#include <glibmm/object.h>

//---Own------------------------------



//---Declaration---------------------------------------------------------------


enum ESerialMessageType
{
    eSerialMessageTypeInvalid,
    eSerialMessageTypeSender,
    eSerialMessageTypeReceiver,
};


class CSerialMessage: public Glib::Object
{
    private:
        ESerialMessageType eSerialMessageType;
        int numFrames;
        int maxFrameSize;
        //int currentFrame;
        int transceived;
        int taskid;

	std::string payload;
        std::string framePayload;

    public:
        CSerialMessage(int _maxFrameSize);

        void initSenderMessage( const Glib::ustring &newPayload, int tid );
        //void initSenderMessage( Glib::RefPtr<const Glib::ustring> &newPayload );

        void initReceiverMessage(int _numFrames);

        bool finished();
        bool popFrame();
        bool pushFrame(int index, int num, const std::string &newFramePayload);
        void discard();

        int getTransceived();

        int numberOfFrames()
        {
            return( numFrames );
        };

        int tid()
        {
            return taskid;
        }

        const std::string &getFramePayload()
        {
            return( framePayload );
        };

        const Glib::ustring getPayload()
        {
            return Glib::ustring(payload);
        };

        gboolean dataLeft();
        gboolean messageFinished();
};


//---fin-----------------------------------------------------------------------
#endif // ? ! SERIAL_MESSAGE_H
