#ifndef MONITOR_H
#define MONITOR_H
//-----------------------------------------------------------------------------
///
/// \brief  single monitor watchig for something
///
///         See implementation for further details
///
/// \date   [20161205] File created
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

//#include <stdio.h>
//#include <string>
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <glibmm/object.h>


//---Own------------------------------

#ifdef DEBUG_CLASSES
    #include "debug.h"
#endif


//---Declaration---------------------------------------------------------------


#define ALARM_VALUE_INVALID LONG_MAX

enum EAlarmDirection
{
    eAlarmDirectionNone,
    eAlarmDirectionAppear,
    eAlarmDirectionDisappear,
};


enum EAlarmState
{
    eAlarmStateOn,
    eAlarmStateOff,
};


enum EAlarmSlopeType
{
    eAlarmSlopeTypeNone = 0 ,       // Alarm disabled
    eAlarmSlopeTypeRising,          // Alarm when value is above trigger value
    eAlarmSlopeTypeFalling,         // Alarm when value is below trigger value
};


class Monitor: public Glib::Object
                #ifdef DEBUG_CLASSES
                , public Debug <Monitor>
                #endif
{
    private:
        Glib::ustring name;
        long appearTriggerValue, disappearTriggerValue;
        EAlarmSlopeType eAlarmSlopeType;
        bool valid;

        EAlarmState eAlarmState;

    public:
        Monitor(const Glib::ustring &_name);
        static Glib::RefPtr <Monitor>create( const Glib::ustring &name );

         sigc::signal<void, EAlarmDirection, Monitor &> alarm;

        virtual void updateValues();
        virtual Glib::ustring getLogString();
        //virtual long getTriggerValue();

         bool setValid(bool _valid);
         bool isValid();

        const Glib::ustring &getName();

        void setAlarmThresholds( long _appearValue, long _disappearValue = ALARM_VALUE_INVALID
                            , EAlarmSlopeType = eAlarmSlopeTypeRising );

        void evaluateAlarms();
        void updateAlarmState(EAlarmState newAlarmState);

        virtual long getTriggerValue();
        long getAppearTriggerValue();
};


//-----------------------------------------------------------------------------
#endif // ? ! MONITOR_H
