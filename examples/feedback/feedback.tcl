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
    # Serial transmitter and receiver Net
    Net ser_rxD0
    Net ser_txD0

    #create a Pin for serial in and serial out of the LCD-board
    ExtPin exttxD0 $Pin_PULLUP $ui "txD0" ".x"
    ExtPin extrxD0 $Pin_PULLUP $ui "rxD0" ".x"

    #create a serial in and serial out component
    SerialRx mysrx $ui "serialRx0" ".x"
    SerialRxBasic_SetBaudRate mysrx 9600
    SerialTx mystx $ui "serialTx0" ".x"
    SerialTxBuffered_SetBaudRate mystx 9600

    # wire the serial display receiver
    ser_rxD0 Add [AvrDevice_GetPin $dev1 "E1"]
    ser_rxD0 Add extrxD0
    ser_rxD0 Add [SerialRxBasic_GetPin mysrx "rx"]

    # wire the serial display transmitter
    #ser_txD0 Add [AvrDevice_GetPin $dev1 "E0"]
    #ser_txD0 Add exttxD0
    ser_txD0 Add [SerialTxBuffered_GetPin mystx "tx"]
  }

  GdbCommands {
    if { ! [info exists f] } {
      error "File handle required"
    }
  }
}
