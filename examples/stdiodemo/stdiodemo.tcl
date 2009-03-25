#
#  Stdio Demo user extension set for simulavr.tcl
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
    ser_txD0 Add [AvrDevice_GetPin $dev1 "E0"]
    ser_txD0 Add exttxD0
    ser_txD0 Add [SerialTxBuffered_GetPin mystx "tx"]

    #create an LCD name mylcd
    Lcd mylcd $ui "lcd0" ".x"
    $sc AddAsyncMember  mylcd

    #Create for the Pins d0 - d4, e, r, c a Net and
    #connect the LCD pins to the AVR pins
    # "r" = Read / Write
    # "c" = command / Data
    # "e" = Enable

    Net E
    E Add [Lcd_GetPin mylcd "e"]
    E Add [AvrDevice_GetPin $dev1 "C7"]

    Net RW
    RW Add [Lcd_GetPin mylcd "r"]
    RW Add [AvrDevice_GetPin $dev1 "C6"]

    Net CD
    CD Add [Lcd_GetPin mylcd "c"]
    CD Add [AvrDevice_GetPin $dev1 "C5"]

    Net D3
    D3 Add [Lcd_GetPin mylcd "d3"]
    D3 Add [AvrDevice_GetPin $dev1 "C3"]

    Net D2
    D2 Add [Lcd_GetPin mylcd "d2"]
    D2 Add [AvrDevice_GetPin $dev1 "C2"]

    Net D1
    D1 Add [Lcd_GetPin mylcd "d1"]
    D1 Add [AvrDevice_GetPin $dev1 "C1"]

    Net D0
    D0 Add [Lcd_GetPin mylcd "d0"]
    D0 Add [AvrDevice_GetPin $dev1 "C0"]
  }

  GdbCommands {
    if { ! [info exists f] } {
      error "File handle required"
    }
    puts $f "break lcd_init"
  }
}
