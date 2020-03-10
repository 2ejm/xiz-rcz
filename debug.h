#ifndef DEBUG_H
#define DEBUG_H
//-----------------------------------------------------------------------------
///
/// \brief  debug class
///
///         Count instantiations via constructor calls.
///         May help to find potential memory leaks.
///
/// \date   [20161205] File created
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

#include <typeinfo>

//---Own------------------------------


//---Declaration---------------------------------------------------------------



template <class T> class
        Debug
{
    private:


    public:
        static int total_instantiations;
        static int open_instantiations;

        Debug()
        {
            total_instantiations++;
            open_instantiations++;
        }

        ~Debug()
        {
            open_instantiations--;
        }

        static void debugInfo()
        {
            printf("class '%s':\n", typeid(T).name());
            printf("   total_instantiations: %d\n", total_instantiations);
            printf("   open_instantiations:  %d\n", open_instantiations);
        }
};


template <class T> int Debug<T>::open_instantiations=0;
template <class T> int Debug<T>::total_instantiations=0;


//-----------------------------------------------------------------------------
#endif // ? ! DEBUG_H
