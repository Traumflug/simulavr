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
    # Serial receiver net (tx from AVR, rx to outside world)
    Net ser_rxD0
    SerialRx mysrx $ui "serialRx0" ".x"
    SerialRxBasic_SetBaudRate mysrx 9600
    ser_rxD0 Add [SerialRxBasic_GetPin mysrx "rx"]
    ser_rxD0 Add [AvrDevice_GetPin $dev1 "E1"]
    $sc Add mysrx

    # Serial receiver net (rx to AVR, tx from outside world)
    #Net ser_txD0
    #SerialTx mystx $ui "serialTx0" ".x"
    #SerialTxBuffered_SetBaudRate mystx 9600
    ##ser_txD0 Add [SerialTxBuffered_GetPin mystx "txData"]
    #ser_txD0 Add [SerialTxBuffered_GetPin mystx "tx"]
    #ser_txD0 Add [AvrDevice_GetPin $dev1 "E0"]
    #$sc Add mystx

  }

  GdbCommands {
    if { ! [info exists f] } {
      error "File handle required"
    }
  }
}
