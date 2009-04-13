#
#  examples/feedback user extension set for simulavr.tcl
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

    #create a serial out (from AVR) component
    SerialRx mysrx $ui "serialRx0" ".x"
    SerialRxBasic_SetBaudRate mysrx 9600
    SerialRxBasic_SetHexOutput mysrx 1
    #create a serial in (to AVR) component
    SerialTx mystx $ui "serialTx0" ".x"
    SerialTxBuffered_SetBaudRate mystx 9600

    # wire the serial receiver and "to byte" device
    ser_rxD0 Add [AvrDevice_GetPin $dev1 "E1"]
    ser_rxD0 Add [SerialRxBasic_GetPin mysrx "rx"]
    # If you want to see bit transitions in the "feedback"
    # create a Pin for serial in and serial out of "the board" into "feedback"
    #ExtPin extrxD0 $Pin_PULLUP $ui "rxD0" ".x"
    #ser_rxD0 Add extrxD0

    # wire the serial transmitter and "from byte" device
    ser_txD0 Add [AvrDevice_GetPin $dev1 "E0"]
    ser_txD0 Add [SerialTxBuffered_GetPin mystx "tx"]
    # If you want to drive bit transitions from the "feedback"
    #ExtPin exttxD0 $Pin_PULLUP $ui "txD0" ".x"
    #ser_txD0 Add exttxD0

    # Analog Support
    set netAref [new_Net]
    $netAref Add [AvrDevice_GetPin $dev1 "AREF"]
    set aRef [new_AdcAnalogPin]
    $aRef SetOutState $Pin_ANALOG
    $netAref Add $aRef
    $aRef setAnalogValue 5000000

    set netF0   [new_Net]
    $netF0 Add [AvrDevice_GetPin $dev1 "F0"]

    set extAdc0 [new_ExtAnalogPin 0 $ui "adc0" ".x"]
    UserInterface_AddExternalType $ui "adc0" $extAdc0
    $netF0 Add $extAdc0
  }

  GdbCommands {
    if { ! [info exists f] } {
      error "File handle required"
    }
  }
}
