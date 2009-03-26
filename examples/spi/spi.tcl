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

    set netB0	[new_Net]
    set netB1	[new_Net]
    set netB2	[new_Net]
    set netB3	[new_Net]

    [AvrDevice_GetPin $dev1 "B0"] SetOutState $Pin_PULLUP

    $netB0 Add [AvrDevice_GetPin $dev1 "B0"]
    $netB1 Add [AvrDevice_GetPin $dev1 "B1"]
    $netB2 Add [AvrDevice_GetPin $dev1 "B2"]
    $netB3 Add [AvrDevice_GetPin $dev1 "B3"]

    SpiSource spiSource "spidata" $netB0 $netB1 $netB2
    SpiSink spiSink $netB0 $netB1 $netB3
    set pinMonPinNameStr	"A0"
    set pinMonPinDescStr	"PORTA0"
    set pinMonPinHighStr	"NEGATE"
    set pinMonPinLowStr	"ASSERT"

    # This line did not work since the string "constants" are
    # apparently de-allocated after the constructor, and the
    # PinMonitor class keeps the pointers, not a copy to
    # the strings!
    #PinMonitor pinmon $dev1 "A0" "PORTA0" "NEGATE" "ASSERT"

    # This works because the variables do not get de-allocated.
    PinMonitor pinmon $dev1 $pinMonPinNameStr $pinMonPinDescStr $pinMonPinHighStr $pinMonPinLowStr

    set netF0	[new_Net]
    set netAref	[new_Net]

    $netF0 Add [AvrDevice_GetPin $dev1 "F0"]

    AdcPin	adcPinSource "anadata" $netF0

    $netAref Add [AvrDevice_GetPin $dev1 "AREF"]

    set aRef [new_AdcAnalogPin]

    $aRef SetOutState $Pin_ANALOG

    $netAref Add $aRef

    $aRef setAnalogValue 5000000

    $sc Add spiSource
    $sc Add spiSink
    $sc Add adcPinSource
  }

  Gui {
  }

  GdbCommands {
    if { ! [info exists f] } {
      error "File handle required"
    }
  }
}
