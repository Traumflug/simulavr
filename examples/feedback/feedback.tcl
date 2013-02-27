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

    ######################## UART 0
    # Serial transmitter and receiver Net
    Net ser_rxD0
    Net ser_txD0

    #create a serial out (from AVR) component (we Rx bytes)
    SerialRx mysrx $ui "serialRx0" ".x"
    SerialRxBasic_SetBaudRate mysrx 9600
    SerialRxBasic_SetHexOutput mysrx 1
    #create a serial in (to AVR) component (we Tx bytes)
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


    ######################## UART 1
    # Serial transmitter and receiver Net
    Net ser_rxD1
    Net ser_txD1

    #create a serial out (from AVR) component
    SerialRx mysrx1 $ui "serialRx1" ".x"
    SerialRxBasic_SetBaudRate mysrx1 19200
    SerialRxBasic_SetHexOutput mysrx1 1
    #create a serial in (to AVR) component
    SerialTx mystx1 $ui "serialTx1" ".x"
    SerialTxBuffered_SetBaudRate mystx1 19200

    # wire the serial receiver and "to byte" device
    ser_rxD1 Add [AvrDevice_GetPin $dev1 "D3"]
    ser_rxD1 Add [SerialRxBasic_GetPin mysrx1 "rx"]
    # If you want to see bit transitions in the "feedback"
    # create a Pin for serial in and serial out of "the board" into "feedback"
    #ExtPin extrxD1 $Pin_PULLUP $ui "rxD1" ".x"
    #ser_rxD1 Add extrxD1

    # wire the serial transmitter and "from byte" device
    ser_txD1 Add [AvrDevice_GetPin $dev1 "D2"]
    ser_txD1 Add [SerialTxBuffered_GetPin mystx1 "tx"]
    # If you want to drive bit transitions from the "feedback"
    #ExtPin exttxD1 $Pin_PULLUP $ui "txD1" ".x"
    #ser_txD1 Add exttxD1

    # Analog Support
    set netAref [new_Net]
    $netAref Add [AvrDevice_GetPin $dev1 "AREF"]
    set aRef [new_AdcAnalogPin]
    $aRef SetOutState $Pin_ANALOG
    $netAref Add $aRef
    $aRef setAnalogValue 5.0

    # F0/ADC0
    if { ${verbose} == 1 } { puts "Adding ADC0" }
    set netF0   [new_Net]
    $netF0 Add [AvrDevice_GetPin $dev1 "F0"]
    set extAdc0 [new_ExtAnalogPin 0 $ui "adc0" ".x"]
    UserInterface_AddExternalType $ui "adc0" $extAdc0
    $netF0 Add $extAdc0

    if { ${verbose} == 1 } { puts "Adding ADC1" }
    # F1/ADC1
    set netF1   [new_Net]
    $netF1 Add [AvrDevice_GetPin $dev1 "F1"]
    set extAdc1 [new_ExtAnalogPin 0 $ui "adc1" ".x"]
    UserInterface_AddExternalType $ui "adc1" $extAdc1
    $netF1 Add $extAdc1

    # F2/ADC2
    if { ${verbose} == 1 } { puts "Adding ADC2" }
    set netF2   [new_Net]
    $netF2 Add [AvrDevice_GetPin $dev1 "F2"]
    set extAdc2 [new_ExtAnalogPin 0 $ui "adc2" ".x"]
    UserInterface_AddExternalType $ui "adc2" $extAdc2
    $netF2 Add $extAdc2

    # F3/ADC3
    if { ${verbose} == 1 } { puts "Adding ADC3" }
    set netF3   [new_Net]
    $netF3 Add [AvrDevice_GetPin $dev1 "F3"]
    set extAdc3 [new_ExtAnalogPin 0 $ui "adc3" ".x"]
    UserInterface_AddExternalType $ui "adc3" $extAdc3
    $netF3 Add $extAdc3

    # F4/ADC4
    if { ${verbose} == 1 } { puts "Adding ADC4" }
    set netF4   [new_Net]
    $netF4 Add [AvrDevice_GetPin $dev1 "F4"]
    set extAdc4 [new_ExtAnalogPin 0 $ui "adc4" ".x"]
    UserInterface_AddExternalType $ui "adc4" $extAdc4
    $netF4 Add $extAdc4

    # F5/ADC5
    if { ${verbose} == 1 } { puts "Adding ADC5" }
    set netF5   [new_Net]
    $netF5 Add [AvrDevice_GetPin $dev1 "F5"]
    set extAdc5 [new_ExtAnalogPin 0 $ui "adc5" ".x"]
    UserInterface_AddExternalType $ui "adc5" $extAdc5
    $netF5 Add $extAdc5

    # F6/ADC6
    if { ${verbose} == 1 } { puts "Adding ADC6" }
    set netF6   [new_Net]
    $netF6 Add [AvrDevice_GetPin $dev1 "F6"]
    set extAdc6 [new_ExtAnalogPin 0 $ui "adc6" ".x"]
    UserInterface_AddExternalType $ui "adc6" $extAdc6
    $netF6 Add $extAdc6

    # F7/ADC7
    if { ${verbose} == 1 } { puts "Adding ADC7" }
    set netF7   [new_Net]
    $netF7 Add [AvrDevice_GetPin $dev1 "F7"]
    set extAdc7 [new_ExtAnalogPin 0 $ui "adc7" ".x"]
    UserInterface_AddExternalType $ui "adc7" $extAdc7
    $netF7 Add $extAdc7
  }

  GdbCommands {
    if { ! [info exists f] } {
      error "File handle required"
    }
  }
}
