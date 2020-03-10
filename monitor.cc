//-----------------------------------------------------------------------------
///
/// \brief  single monitor watchig for something
///
///         Each monitor has to be updated cyclic by calling updateValues()
///         and evaluateAlarms().
///         When an the trigger value is exceeded an alarm is emited with
///         direction "appear".
///         When the trigger value is not exceede any more an alarm is emited
///         with direction "disappear".
///
///         Fuzzy logic is planned but not implemented yet because there is no
///         use case at the moment.
///         Fuzzy logic would work with two different threshold values for
///         "appear" and "disappear".
///
/// \date   [20161205] File created
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------


//---Own------------------------------

#include "monitor.h"
#include "log.h"


//---Implementation------------------------------------------------------------


Monitor::Monitor(const Glib::ustring &_name)
    :name(_name)
{
    eAlarmSlopeType=eAlarmSlopeTypeNone;
    valid=false;
    eAlarmState=eAlarmStateOff;
};


Glib::RefPtr <Monitor>Monitor::create( const Glib::ustring &name ) /* static  */
{
    return ( Glib::RefPtr <Monitor>( new Monitor(name) ) );
}


void Monitor::updateValues() /* virtual */
{
    return;
}


Glib::ustring Monitor::getLogString() /* virtual */
{
    return( "### Dummy monitor!" );
}


const Glib::ustring &Monitor::getName()
{
    return(name);
}


bool Monitor::setValid(bool _valid)
{
    return( ( valid = _valid ) );
}


bool Monitor::isValid()
{
    return( valid );
}


void Monitor::setAlarmThresholds( long _appearValue, long _disappearValue /* = ALARM_VALUE_INVALID */
                    , EAlarmSlopeType _eAlarmSlopeType /* = eAlarmSlopeTypeUp */ )
{
    appearTriggerValue=_appearValue;

    // Skip fuzzy logic and use same trigger values for appear and disappear?
    if ( _disappearValue== ALARM_VALUE_INVALID )
        disappearTriggerValue=_appearValue;
    else
        disappearTriggerValue=_disappearValue;

    // Check for validity of trigger values
    if( _eAlarmSlopeType == eAlarmSlopeTypeRising )
        if( disappearTriggerValue > appearTriggerValue )
        {
            lError("### Error: disappear-trigger-value can not be bigger than apper-trigger-value!\n");
            disappearTriggerValue=appearTriggerValue;
        }

    if( _eAlarmSlopeType == eAlarmSlopeTypeFalling )
        if( disappearTriggerValue < appearTriggerValue )
        {
            lError("### Error: disappear-trigger-value can not be bigger than apper-trigger-value!\n");
            disappearTriggerValue=appearTriggerValue;
        }


    eAlarmSlopeType=_eAlarmSlopeType;
}


void Monitor::evaluateAlarms()
{
    EAlarmState newAlarmState=eAlarmState;

    if( eAlarmSlopeType == eAlarmSlopeTypeNone )
    {
        lHighDebug("      Alarm inactive\n");
        return;
    }

    if( ! isValid() )
    {
        lHint("evaluateAlarms(): Values not valid\n");
        return;
    }

    // Fuzzy Logic:
    if( newAlarmState == eAlarmStateOff)
    {
        switch(eAlarmSlopeType)
        {
            case eAlarmSlopeTypeRising:
                if( getTriggerValue() > appearTriggerValue )
                    newAlarmState=eAlarmStateOn;
                break;
            case eAlarmSlopeTypeFalling:
                if( getTriggerValue() < appearTriggerValue )
                    newAlarmState=eAlarmStateOn;
                break;
            default:
                // Should not happen, no alarm.
                lFatal("No slope defined");
                break;
        }
    }
    else // -> newAlarmState == eAlarmStateOn
    {
        switch(eAlarmSlopeType)
        {
            case eAlarmSlopeTypeRising:
                if( getTriggerValue() < disappearTriggerValue )
                    newAlarmState=eAlarmStateOff;
                break;
            case eAlarmSlopeTypeFalling:
                if( getTriggerValue() > disappearTriggerValue )
                    newAlarmState=eAlarmStateOff;
                break;
            default:
                // Should not happen, no alarm.
                lFatal("No slope defined");
                break;
        }
    }

    updateAlarmState(newAlarmState);

    return;
}


/// \brief  The alarm state has been calculated. take the consequence here
///
///         It checks the previous alarm state and evaluates if signals
///         (appear /  disappear) have to be send.
void Monitor::updateAlarmState(EAlarmState newAlarmState)
{
    switch(newAlarmState)
    {
        case eAlarmStateOn:
            if( eAlarmState == eAlarmStateOff )
                    alarm.emit(eAlarmDirectionAppear, *this);
            break;

        case eAlarmStateOff:
            if( eAlarmState == eAlarmStateOn )
                    alarm.emit(eAlarmDirectionDisappear, *this);
            break;

        default:
            throw Glib::ustring::compose("Unknown alarm state: %1", newAlarmState);
            break;
    }

    eAlarmState=newAlarmState;
    return;
}


long Monitor::getTriggerValue() /* virtual */
{
    // nothing, absolutely nothing.
    return( 0 );
}


long Monitor::getAppearTriggerValue()
{
    return( appearTriggerValue );
}

//---fin.----------------------------------------------------------------------
