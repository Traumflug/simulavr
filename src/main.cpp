/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002, 2003   Klaus Rudolph		
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ****************************************************************************
 */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
//#include <signal.h>

#include <map>

#include <sstream>
#include <iostream>
#include <string>
using namespace std;

#include "config.h"

#include "global.h"
#include "avrdevice.h"
#include "at8515.h"
#include "at8515special.h"
#include "atmega128.h"
#include "at4433.h"
#include "gdb.h"
#include "ui.h"
#include "systemclock.h"
#include "lcd.h"
#include "keyboard.h"
#include "trace.h"
#include "scope.h"

#define PRG_WISH "/usr/bin/wish"

//test only!

extern int trace_on;

int main2();
int main4();

int main(int argc, char *argv[]) {
    int c;
    //int digit_optind = 0;
    bool gdbserver_flag=0;
    string filename("unknown");
    string devicename("unknown");
    string tracefilename("unknown");
    int global_gdbserver_port    = 1212;
    int global_gdb_debug         = 0;
    int extended_tests=0;
    int userinterface_flag=0;
    UserInterface *ui;

    trace_on=0;


    while (1) {
        //int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"file", 1, 0, 'f'},
            {"device", 1, 0, 'd'},
            {"gdbserver", 1, 0, 'g'},
            {"trace", 1, 0, 't'},
            {"version",0,0,'v'},
            {0, 0, 0, 0}
        };

        c = getopt_long (argc, argv, "f:d:gGd:p:t:uxzhv", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'u':
                cout << "Run with User Interface at Port 7777" << endl;
                userinterface_flag=1;
                break;

            case 'f':
                cout << "File to load " << optarg << endl;
                filename=optarg;
                break;

            case 'd':
                cout << "Device to simulate " << optarg << endl;
                devicename=optarg;
                break;

            case 'g':
                cout << "Running as gdb-server" << endl;
                gdbserver_flag=1;
                break;

            case 'G':
                cout << "Running with debug informations from gdbserver" << endl;
                global_gdb_debug = 1;
                break;

            case 'p':
                cout << "Running NOT on default port, use instead: " << global_gdbserver_port << endl;
                global_gdbserver_port    = atoi(optarg);
                break;

            case 't':
                cout << "Running in Trace Mode" << endl;
                StartTrace(optarg);
                break;

            case 'x':
                cout << "Running in other main loop for tests only! DO NOT USE" << endl;
                extended_tests=1;
                break;

            case 'z':
                cout << "Running in other main loop for tests only! DO NOT USE" << endl;
                extended_tests=3;
                break;

           case 'v':
              {
              std::cout << "Simulavr++ " << VERSION << std::endl;
              std::cout << "See documentation for copyright and distribution terms" << std::endl;
              std::cout << std::endl;
              exit(0);
              }
              break;

            default:
                cout << "AVR-Simulator" << endl;
                cout << "-u                           run with user interface for external pin handling at port 7777" << endl;
                cout << "-f --file <name>             load elf-file <name> for simulation in simulated target" << endl; 
                cout << "-d --device <device name>    simulate <device name> " << endl;
                cout << "-g --gdbserver               running as gdb-server" << endl;
                cout << "-p  <port>                   change <port> for gdb server to port" << endl;
                cout << "-t --trace <file name>       enable trace outputs into <file name>" << endl;
                cout << endl;
                cout << "Supported devices:" << endl;
                cout << "at90s8515" << endl;
                cout << "at90s4433" << endl;
                cout << "atmega128" << endl;
                cout << endl;

                exit(0);

        }
    }

    if (extended_tests==1) {
        main2();
        //#warning NOW COMPILED FOR IO TEST ONLY NOT NORMAL -X OPERATION
        //main3();
        return 0;
    } 

    if (extended_tests==3) {
        main4();
        return 0;
    }

    /* now we create the device */
    AvrDevice *dev1;
    if (devicename=="unknown") {
        dev1= new AvrDevice_at90s8515;
    } else if (devicename=="at90s8515") {
        dev1= new AvrDevice_at90s8515;
    } else if (devicename=="atmega128") {
        dev1= new AvrDevice_atmega128;
    } else if (devicename=="at90s4433") {
        dev1= new AvrDevice_at90s4433;
    } else {
        cerr << "Not a valid device" << endl;
        exit(0);
    }

    dev1->SetClockFreq(250); //4 Mhz for dummy
    systemClock.Add(dev1);


    if (filename != "unknown" ) {
        dev1->Load(filename.c_str());
    }

    if (userinterface_flag==1) {
        ui=new UserInterface(7777); //if not gdb, the ui will be master controller :-)
    }

    if (gdbserver_flag==1) {
        cout << "Going to gdb..." << endl;
        gdb_interact( dev1, global_gdbserver_port, global_gdb_debug );
    } else {
        systemClock.Add(dev1);

        while (1) { //for ever
            systemClock.Step(0);
        }

    }

}
/*
void ChildExit(int a) {
cout << "Child ends.... so we go away, thank you for using me :-)" << endl;
kill(getpid(), SIGQUIT);
}
*/
#define USE_ANA
#define USE_PWM4
//#define USE_WEICHE8
#define EXPORT_CLKDATA
int main2() {


    //start the wish interpreter with gui application
    pid_t pid= fork();

    char *args[3];
    args[0]=PRG_WISH;
    args[1]="../tests/gui.tcl";
    args[2]=NULL;

    if (pid==0) { //the child
        execve( PRG_WISH , args, environ); //no return from here!
    }
    /*
    //if child dies we also want to exit
    struct sigaction snew;

    snew.sa_handler = ChildExit;
    sigemptyset(&snew.sa_mask);
    snew.sa_flags = 0;
    snew.sa_restorer = NULL; 
    sigaction(SIGCHLD,&snew,NULL);



    int sigaction(int signum,  const  struct  sigaction  *act, struct sigaction *oldact);

*/
    Net clk;                        //the net
    Net data; //all dataRead and dataWrite are connected!

    UserInterface *ui=new UserInterface(7777);
#ifdef USE_PWM4
    AvrDevice *dev1= new AvrDevice_at90s8515;
    dev1->Load("pwm4.o.go"); //fahrspannungserzeuger
    //dev1->SetClockFreq(75);    
    dev1->SetClockFreq(257);    //4Mhz
    systemClock.Add(dev1);
    
    clk.Add(dev1->GetPin("D2"));  //pwm
    OpenDrain odPwmDataW( dev1->GetPin("D4") ); // pwm data write
    data.Add(&odPwmDataW);
    data.Add(dev1->GetPin("D3"));  //pwm read


    (*ui)<<"frame .pwm4"<<endl;
    (*ui)<<"pack .pwm4" << endl;



    Net bm0;
    ExtPin extbm0(Pin::TRISTATE, ui, "bm0", ".pwm4" );
    bm0.Add(&extbm0);
    bm0.Add(dev1->GetPin("A4"));

    Net bm1;
    ExtPin extbm1(Pin::TRISTATE, ui, "bm1", ".pwm4");
    bm1.Add(&extbm1);
    bm1.Add(dev1->GetPin("A5"));

    Net bm2;
    ExtPin extbm2(Pin::TRISTATE, ui, "bm2", ".pwm4");
    bm2.Add(&extbm2);
    bm2.Add(dev1->GetPin("A6"));

    Net bm3;
    ExtPin extbm3(Pin::TRISTATE, ui, "bm3", ".pwm4");
    bm3.Add(&extbm3);
    bm3.Add(dev1->GetPin("A7"));

    Scope sc0(ui, "pwm_scope", 4, ".pwm4"); //scope with 4 channels

    Net pwm0;
    Net pwm1;
    Net pwm2;
    Net pwm3;

/*    pwm0.Add(sc0.GetPin(0));
    pwm1.Add(sc0.GetPin(1));
    pwm2.Add(sc0.GetPin(2));
    pwm3.Add(sc0.GetPin(3));
    */

    ExtPin ep0 (Pin::TRISTATE, ui, "pwm0", ".pwm4");
    ExtPin ep1 (Pin::TRISTATE, ui, "pwm1", ".pwm4");
    ExtPin ep2 (Pin::TRISTATE, ui, "pwm2", ".pwm4");
    ExtPin ep3 (Pin::TRISTATE, ui, "pwm3", ".pwm4");

    pwm0.Add(&ep0);
    pwm1.Add(&ep1);
    pwm2.Add(&ep2);
    pwm3.Add(&ep3);




    pwm0.Add(dev1->GetPin("A0"));
    pwm1.Add(dev1->GetPin("A1"));
    pwm2.Add(dev1->GetPin("A2"));
    pwm3.Add(dev1->GetPin("A3"));

    //dauerzugbeleuchtung ausgang
    Net npwmdz0;
    Net npwmdz1;
    Net npwmdz2;
    Net npwmdz3;

    ExtPin pwmdz0(Pin::TRISTATE, ui, "pwmdz0", ".pwm4");
    ExtPin pwmdz1(Pin::TRISTATE, ui, "pwmdz1", ".pwm4");
    ExtPin pwmdz2(Pin::TRISTATE, ui, "pwmdz2", ".pwm4");
    ExtPin pwmdz3(Pin::TRISTATE, ui, "pwmdz3", ".pwm4");

    npwmdz0.Add(&pwmdz0);
    npwmdz1.Add(&pwmdz1);
    npwmdz2.Add(&pwmdz2);
    npwmdz3.Add(&pwmdz3);

    npwmdz0.Add(dev1->GetPin("C0"));
    npwmdz1.Add(dev1->GetPin("C1"));
    npwmdz2.Add(dev1->GetPin("C2"));
    npwmdz3.Add(dev1->GetPin("C3"));


    //Gleichrichter Ausgang
    Net npwmdc0;
    Net npwmdc1;
    Net npwmdc2;
    Net npwmdc3;

    ExtPin pwmdc0(Pin::TRISTATE, ui, "pwmdc0", ".pwm4");
    ExtPin pwmdc1(Pin::TRISTATE, ui, "pwmdc1", ".pwm4");
    ExtPin pwmdc2(Pin::TRISTATE, ui, "pwmdc2", ".pwm4");
    ExtPin pwmdc3(Pin::TRISTATE, ui, "pwmdc3", ".pwm4");

    npwmdc0.Add(&pwmdc0);
    npwmdc1.Add(&pwmdc1);
    npwmdc2.Add(&pwmdc2);
    npwmdc3.Add(&pwmdc3);

    npwmdc0.Add(dev1->GetPin("C4"));
    npwmdc1.Add(dev1->GetPin("C5"));
    npwmdc2.Add(dev1->GetPin("C6"));
    npwmdc3.Add(dev1->GetPin("C7"));




#endif
    ostringstream os;
    os << "frame .x.l" << endl;
    os << "pack .x.l" << endl;
    os << "button .x.l.b -text \"NIX\"" << endl;
    os << "pack .x.l.b -side left" << endl;
    ui->Write(os.str());

#ifdef USE_WEICHE8
    AvrDevice *dev5= new AvrDevice_at90s8515;
    dev5->Load("weiche8.o.go"); //Master
    dev5->SetClockFreq(125);    //125 ns per Cycle -> 8Mhz
    systemClock.Add(dev5);

    OpenDrain odWeicheDataW( dev5->GetPin("D4") ); //weiche data write
    data.Add(&odWeicheDataW);
    clk.Add(dev5->GetPin("D2")); //weiche
    data.Add(dev5->GetPin("D3")); //weiche data read
#endif

    AvrDevice *dev2= new AvrDevice_atmega128;
    dev2->Load("test.o.go"); //Master
    //dev2->SetClockFreq(200);    //200 ns per Cycle -> 5Mhz
    //dev2->SetClockFreq(250);    //200 ns per Cycle -> 5Mhz
    dev2->SetClockFreq(272);     //3.66 Mhz
    systemClock.Add(dev2);

    OpenDrain odMasterDataW( dev2->GetPin("B4") ); //master
    data.Add(&odMasterDataW);   //master
    OpenDrain odMasterClk(dev2->GetPin("B2")); //reverse the master
    clk.Add(&odMasterClk);
    data.Add(dev2->GetPin("B3")); //master read


    AvrDevice *dev3= new AvrDevice_at90s8515special;
    dev3->Load("monispecial.o.go"); //Debugger
    //dev3->SetClockFreq(200);    //200 ns per Cycle -> 5Mhz
    dev3->SetClockFreq(250);    //200 ns per Cycle -> 5Mhz
    systemClock.Add(dev3);
    
    clk.Add(dev3->GetPin("D2"));    //moni
    data.Add(dev3->GetPin("D3")); //moni read
    OpenDrain odMoniDataW( dev3->GetPin("D4") ); //moni
    data.Add(&odMoniDataW); //moni


#ifdef USE_ANA
    AvrDevice *dev4= new AvrDevice_at90s4433;
    dev4->Load("ana.o.go"); //analog control 
    dev4->SetClockFreq(250); //4MHz
    systemClock.Add(dev4);

    clk.Add(dev4->GetPin("D2"));  //ana
    OpenDrain odAnaDataW( dev4->GetPin("D4") ); // ana data write
    data.Add(&odAnaDataW);
    data.Add(dev4->GetPin("D3")); //ana read

    (*ui) << "frame .ana" << endl;
    (*ui) << "pack .ana" << endl;

    ExtAnalogPin evref( INT_MAX, ui, "vref", ".ana");
    ExtAnalogPin ev0( 0, ui, "v0", ".ana");
    ExtAnalogPin ev1( 0, ui, "v1", ".ana");

    Net vref;
    vref.Add(&evref);
    vref.Add( dev4->GetPin("Y0"));

    Net v0;
    v0.Add(&ev0);
    v0.Add( dev4->GetPin("C0"));

    Net v1;
    v1.Add(&ev1);
    v1.Add( dev4->GetPin("C1"));


    Net dc0;
    Net dc1;
    Net dzb0;
    Net dzb1;

    ExtPin edc0(Pin::TRISTATE, ui, "anadc0", ".ana");
    ExtPin edc1(Pin::TRISTATE, ui, "anadc1", ".ana");

    ExtPin edzb0(Pin::TRISTATE, ui, "anadzb0", ".ana");
    ExtPin edzb1(Pin::TRISTATE, ui, "anadzb1", ".ana");

    dc0.Add(&edc0);
    dc0.Add(dev4->GetPin("C2"));

    dc1.Add(&edc1);
    dc1.Add(dev4->GetPin("C4"));

    dzb0.Add(&edzb0);
    dzb0.Add(dev4->GetPin("C3"));

    dzb1.Add(&edzb1);
    dzb1.Add(dev4->GetPin("C5"));


#endif

    (*ui) << "frame .master" << endl;
    (*ui) << "pack .master" << endl;

    systemClock.AddAsyncMember(ui);

    Lcd lcd(ui, "lcd1", ".master");
    systemClock.AddAsyncMember(&lcd);

#ifdef EXPORT_CLKDATA
    ExtPin extClk(Pin::TRISTATE, ui, "clk", ".master");      //this pin is the external representation via the ui (ui.cpp)
    clk.Add(&extClk);               //now add this pin to the net (that is the first nice action here
#endif
    Pin extPullUpClk(Pin::PULLUP);  //create a pull up
    clk.Add(&extPullUpClk);         //and add it to the net here



    //?????? 
    //the pins are calculated many times here 
    //we need a redesign!
    //and we net better names here (-> UiPin?) which is allways visible from external UI (generate a UI with port and 
    //give the constructor from the UiPin the reference of the ui
    //no pin lists for calculation needed anymore... that is allready handled in the ui (ui knows all pins to handle)
    //




#ifdef EXPORT_CLKDATA
    ExtPin extData(Pin::TRISTATE, ui, "data", ".master");
    data.Add(&extData);
#endif

    Pin extPullupPin(Pin::PULLUP);
    data.Add(&extPullupPin);          //we use external Pull-UP!

    //adding the lcd stuff now:#
    Net lcd_d0;
    lcd_d0.Add(dev2->GetPin("A4"));
    lcd_d0.Add(lcd.GetPin("d0"));

    Net lcd_d1;
    lcd_d1.Add(dev2->GetPin("A5"));
    lcd_d1.Add(lcd.GetPin("d1"));

    Net lcd_d2;
    lcd_d2.Add(dev2->GetPin("A6"));
    lcd_d2.Add(lcd.GetPin("d2"));

    Net lcd_d3;
    lcd_d3.Add(dev2->GetPin("A7"));
    lcd_d3.Add(lcd.GetPin("d3"));

    Net lcd_enable;
    lcd_enable.Add(dev2->GetPin("A3"));
    lcd_enable.Add(lcd.GetPin("e"));

    Net lcd_readWrite;
    lcd_readWrite.Add(dev2->GetPin("A2"));
    lcd_readWrite.Add(lcd.GetPin("r"));

    Net lcd_command;
    lcd_command.Add(dev2->GetPin("A1"));
    lcd_command.Add(lcd.GetPin("c"));


    Net key_clk;
    Net key_data;

    ExtPin extKbdClk(Pin::TRISTATE, ui, "kbdClk",".master");


    ExtPin extKbdData(Pin::TRISTATE, ui, "kbdData",".master");



    //now add a keyboard there :-)
    Keyboard kbd(ui, "kbd1");
    kbd.SetClockFreq(40000); //30..50 us is normal clocking rate so we use midrange device here :-)
    systemClock.Add(&kbd);

    key_data.Add(dev2->GetPin("D4"));
    key_data.Add(kbd.GetPin("data"));
    key_data.Add(&extKbdData);
    key_clk.Add(dev2->GetPin("D3"));
    key_clk.Add(kbd.GetPin("clk"));
    key_clk.Add(&extKbdClk);

    systemClock.Endless();

    cout << "End of simulation" << endl;
    return 0;
}

int main4() {

    cout << "Starting with gui for main3" << endl;
    UserInterface *ui=new UserInterface(7777);
    systemClock.AddAsyncMember(ui);

    Net neta;

    AvrDevice *dev1= new AvrDevice_at90s8515;

    dev1->Load("../examples/pinnet/pinnet");
    dev1->SetClockFreq(250);

    neta.Add(dev1->GetPin("B0"));
    neta.Add(dev1->GetPin("D0"));

    ExtPin ep(Pin::TRISTATE, ui, "bO", ".x");
    neta.Add(&ep);

    systemClock.Add(dev1);
    cout << "Device setup complete" << endl;
    while(1) {
        systemClock.Step(0);
    }




    return 0;
}



