//----------------------------------------------------------------------------
///
/// \file   helpers.cc
///
/// \brief  Helper functions for visux
///
///         nothing concrete related to visux but used by it.
///
//----------------------------------------------------------------------------


//---Includes-----------------------------------------------------------------


//---General-------------------------

#include <stdio.h>
#include <glibmm/ustring.h>

//---Own-----------------------------


//---Implementation-----------------------------------------------------------


/// \brief  print hexdump of memory
void dump(const char *buffer, int size)
{
    int i1;

    for(i1=0; i1<size; i1++)
    {

        if(i1 && (! (i1%8) ) )
            printf("\n");

        if( (! (i1%8) ) )
            printf("%04X: ", i1);

        printf("[%02X] ", (int)(unsigned char)buffer[i1]);
    }

    if( (i1%8) )
        printf("\n");
}


/// \brief  returns length of string with limited maximum
///
///         Better use this function than something like
///             MAX( length(string), maxLength)
///         because it actually stops after "maxLength". Used in cases where
///         c-strings might not be zero terminated.
int limitedLength(const char *buffer, int maxLength)
{
    int length=0;

    while( buffer[length] && (length<maxLength) )
        length++;

    return(length);
}


/// \brief  appends string with limited maximum size to an ustring
///
///         Used in cases where c-strings might not be zero terminated.
int podString(Glib::ustring &dest, const char *buffer, int maxLength)
{
    int len;
    dest.clear();
    dest.append( buffer, len=limitedLength(buffer, maxLength ) );

    return(len);
}


//---fin.---------------------------------------------------------------------
