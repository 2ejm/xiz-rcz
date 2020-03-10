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

#include <glib.h>
#include <stdio.h>
#include <string.h>         // memcpy

//---Own------------------------------

#include "serial_message.h"
#include "crc32.h"
#include "log.h"


//---Forward declarations------------------------------------------------------



//---Implementation------------------------------------------------------------


CSerialMessage::CSerialMessage(int _maxFrameSize)
{
    eSerialMessageType=eSerialMessageTypeInvalid;
    numFrames=0;
    maxFrameSize=_maxFrameSize;
    transceived=0;

    return;
}


/// \brief  Drops current state of message
void CSerialMessage::discard()
{
    payload.erase();
    framePayload.erase();

    transceived=numFrames=0;

    return;
}


/// \brief  Get the number of frames that have been pushed or poped
///
///         Caution: This can be used as frame index when sending a Frame but
///         you have to substract 1
int CSerialMessage::getTransceived()
{
    return( transceived );
};


void CSerialMessage::initSenderMessage( const Glib::ustring &newPayload, int tid )
{
    discard();
    payload=newPayload;
    numFrames= payload.size() / maxFrameSize;
    if( payload.size() % maxFrameSize )
        numFrames++;
    taskid = tid;
}


void CSerialMessage::initReceiverMessage(int _numFrames)
{
    discard();
    numFrames=_numFrames;
}


/// \brief  prepare a new frame to send
///
///         calculates a new frame to be send.
bool CSerialMessage::popFrame()
{

    if( ! payload.length() )
    {
        lError("Error: pop: no data left\n");
        return(true);
    }

    framePayload = payload.substr(0, maxFrameSize);
    payload.erase(0, framePayload.length() );
    transceived++;

    lHighDebug("POP: frame payload:\"%s\"; remaining payload:\"%s\"\n",
           framePayload.c_str(),
           payload.c_str()
           );

    return(false);
}


/// \brief  Frame was received, add it to messge
///
///         Caution: Message has to be initialized correctly in order to work
///         Properly.
bool CSerialMessage::pushFrame(int index, int num, const std::string &newFramePayload)
{
    (void)index;
    (void)num;
    payload+=newFramePayload;
    transceived++;

    return(false);
}


/// \brief  checks if there is any payload left to be send in current message
///
/// \return true:   frame(s) left to be send
///         false:  No data left to be send.
gboolean CSerialMessage::dataLeft()
{
    if ( ( transceived < numFrames ) && ( ! payload.length() ) )
        lError("### Error: frames to be send but no data\n");

    if ( ( transceived >= numFrames ) && ( payload.length() ) )
        lError("### Error: no frames to be send but data left; transceived: %d; data left: '%s'\n", transceived, payload.c_str() );

    if ( transceived < numFrames )
        return(true);

    return(false);
}


gboolean CSerialMessage::messageFinished()
{
    return(transceived>=numFrames);
}


//---fin.---------------------------------------------------------------------
