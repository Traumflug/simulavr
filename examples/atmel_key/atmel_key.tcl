#
#  Atmel PC Keyboard user extension set for simulavr.tcl
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

    # create the keyboard specific nets
    Net key_clk
    Net key_data

    # For the Atmel specifics
    Net key_runLED
    Net key_TestPin

    # For sending the keyboard data to the display
    Net key_txD0

    # create the keyboard specific pins
    ExtPin extKbdClk  $Pin_TRISTATE $ui "kbdClk" ".x"
    ExtPin extKbdData $Pin_TRISTATE $ui "kbdData" ".x"

    # These both indicators relate to AVR313 AP-Note
    ExtPin extrunLED $Pin_TRISTATE $ui "runLED" ".x"
    ExtPin extTestPin $Pin_TRISTATE $ui "testPin" ".x"

    #ExtPin extrxD0 $Pin_TRISTATE $ui "txD0" ".x"
    ExtPin exttxD0 $Pin_PULLUP $ui "txD0" ".x"

    # now add the dynamic of the keyboard here :-)
    Keyboard kbd $ui "kbd1" ".x"

    #30..50 us is normal clocking rate so we use midrange device here :-)
    Keyboard_SetClockFreq kbd 40000
    $sc Add kbd

    #create some nets which connect the pins
    key_data Add [AvrDevice_GetPin $dev1 "D3"]
    key_data Add [Keyboard_GetPin kbd "data"]
    key_data Add extKbdData

    key_clk Add [AvrDevice_GetPin $dev1 "D0"]
    key_clk Add [Keyboard_GetPin kbd "clk"]
    key_clk Add extKbdClk

    key_runLED Add [AvrDevice_GetPin $dev1 "D5"]
    key_runLED Add extrunLED

    key_TestPin Add [AvrDevice_GetPin $dev1 "B1"]
    key_TestPin Add extTestPin

    SerialRx mysrx $ui "serialRx0" ".x"
    SerialRxBasic_SetBaudRate mysrx 19200

    key_txD0 Add [AvrDevice_GetPin $dev1 "E1"]
    key_txD0 Add exttxD0
    key_txD0 Add [SerialRxBasic_GetPin mysrx "rx"]
  }

  GdbCommands {
    if { ! [info exists f] } {
      error "File handle required"
    }
  }
}
