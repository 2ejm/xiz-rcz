#!/bin/bash
#------------------------------------------------------------------------------
#
# \brief	send test xml to socket
#
#------------------------------------------------------------------------------


# netcat localhost 17001 < testxmls/l1_utf8.xml
# netcat localhost 17001 < testxmls/l1.xml
# netcat localhost 17001 < testxmls/l1.xml
netcat localhost 17001 < testxmls/f1.xml



echo "[$0] fin."


#---fin------------------------------------------------------------------------
