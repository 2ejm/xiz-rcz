#ifndef _HELPERS_H
#define _HELPERS_H
//-----------------------------------------------------------------------------
///
/// \brief  helper functions
///
///         Helper functions for visux.
///         See implementation for further details.
///
/// \date   [20161212] File created
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

//---Own------------------------------


//---Declaration---------------------------------------------------------------

void dump(const char *buffer, int size);
int limitedLength(const char *buffer, int maxLength);
int podString(Glib::ustring &dest, const char *buffer, int maxLength);


//-----------------------------------------------------------------------------
#endif // ? ! _HELPERS_H
