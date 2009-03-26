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

    #
    # Create the nets/wires for the SPI bus interface.
    #
    set netSS	[new_Net]
    set netSCK	[new_Net]
    set netMOSI	[new_Net]
    set netMISO	[new_Net]

    #
    # Set the MOSI pin of the ATMega48 to pullup.
    #
    [AvrDevice_GetPin $dev1 "B2"] SetOutState $Pin_PULLUP

    #
    # Attach the ATMega48 SPI bus pins to their nets.
    #
    $netSS Add [AvrDevice_GetPin $dev1 "B2"]
    $netSCK Add [AvrDevice_GetPin $dev1 "B5"]
    $netMOSI Add [AvrDevice_GetPin $dev1 "B3"]
    $netMISO Add [AvrDevice_GetPin $dev1 "B4"]

    #
    # Create a SPI bus stimulator. This stimulator continuously
    # sends the contents of the "spidata" file to the SPI bus
    # /SS, SCLK, and MOSI.
    #
    SpiSource spiSource "spidata" $netSS $netSCK $netMOSI

    #
    # Create a SPI bus monitor. This monitor watches the
    # state of the /SS, SCLK and MISO SPI signals decodes
    # them, and prints the decoded bytes to stdout.
    #
    SpiSink spiSink $netSS $netSCK $netMISO

    #
    # String variables for the IRQ output PinMonitor
    #
    set irqMonPinNameStr	"B0"
    set irqMonPinDescStr	"PORTB0"
    set irqMonPinHighStr	"NEGATE"
    set irqMonPinLowStr		"ASSERT"

    #
    # This commented line did not work since the string "constants" are
    # apparently de-allocated after the constructor, and the
    # PinMonitor class keeps the pointers, not a copy to
    # the strings!
    #
    #    PinMonitor irqmon $dev1 "B0" "/IOP_IRQ" "NEGATE" "ASSERT"
    #
    # This works, however, because the string variables do not get de-allocated.
    #
    PinMonitor irqmon $dev1 $irqMonPinNameStr $irqMonPinDescStr $irqMonPinHighStr $irqMonPinLowStr

    #
    # Create the nets for the analog inputs.
    #
    set netADC6	[new_Net]
    set netADC7	[new_Net]
    set netC5	[new_Net]
    set netAref	[new_Net]

    #
    # Attach the appropriate ATMega48 analog input pins to their nets.
    #
    $netADC6 Add [AvrDevice_GetPin $dev1 "ADC6"]
    $netADC7 Add [AvrDevice_GetPin $dev1 "ADC7"]
    $netC5 Add [AvrDevice_GetPin $dev1 "C5"]

    #
    # Create the analog simulations for each of the
    # three analog inputs that we are demonstrating.
    #
    AdcPin	adc6PinSource "anadata1" $netADC6
    AdcPin	adc7PinSource "anadata2" $netADC7
    AdcPin	pc5PinSource "anadata3" $netC5

    #
    # Attach the AREF input pin to its nets.
    #
    $netAref Add [AvrDevice_GetPin $dev1 "AREF"]

    #
    # Create an analog input simulation pin for the reference input.
    #
    set aRef [new_AdcAnalogPin]

    #
    # Set the analog voltage reference pin type to ANALOG.
    #
    $aRef SetOutState $Pin_ANALOG

    #
    # Attach the simulated voltage reference pin to the AREF net.
    #
    $netAref Add $aRef

    #
    # Set the value of the voltage reference (5 volts?)
    #
    $aRef setAnalogValue 5000000

    #
    # Add the simulation members to the simulation clock.
    #
    $sc Add spiSource
    $sc Add spiSink
    $sc Add adc6PinSource
    $sc Add adc7PinSource
    $sc Add pc5PinSource
  }

  Gui {
  }

  GdbCommands {
    if { ! [info exists f] } {
      error "File handle required"
    }
  }
}
