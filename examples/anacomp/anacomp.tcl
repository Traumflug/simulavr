#
#  Anacomp user extension set for simulavr.tcl
#
#  $Id$
#

switch ${extensionPoint} {
  Initialization {}

  CPU {
    if { ! [info exists dev1] } {
      error "MCU Device Required"
    }
  }

  Gui {
    if { ! [info exists ui] } {
      error "User Interface Required"
    }

    #create some external pins
    ExtAnalogPin pain0 0 $ui "ain0" ".x"
    ExtAnalogPin pain1 0 $ui "ain1" ".x"
    ExtPin epb $Pin_TRISTATE $ui "->BO" ".x"

    #create some nets which connect the pins 
    Net ain0
    ain0 Add pain0
    ain0 Add [AvrDevice_GetPin $dev1 "D6"]

    Net ain1
    ain1 Add pain1
    ain1 Add [AvrDevice_GetPin $dev1 "D7"]

    Net portb
    portb Add epb
    portb Add [AvrDevice_GetPin $dev1 "B0"]
  }

  GdbCommands {
    if { ! [info exists f] } {
      error "File handle required"
    }
  }
}
