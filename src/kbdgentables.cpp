#include <map>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

void outNumberToScancode(ostream &target, int set, int number, const string &str) {
    static int objcnt=0;
    objcnt++;

    target << "static const int object"<<objcnt<<"[] = { ";

    istringstream is(str);

    int cnt=0;
    do {
        char val[5];
        char dummy; //overread ','

        is.get(val, 5, ',');
        if (is.fail()) break;
        target << "0x00" << val;
        is >> dummy;
        if (is.fail()) break;
        target << ",";
        cnt++;



    } while (!is.eof());

    //for (;cnt<MAX_CHAR_PER_KEY; cnt++) {
    target << ",0xffff";
    //}




    target << "};" << endl;

    target << "keynumberToScancode" << set << "[" << number << "]= object"<<objcnt<<";"<< endl;

}

int main() {
    ifstream xToNumber("./xcode_to_keynumber.dat");
    ifstream keynumberToScancode("./keynumber_to_scancode.dat"); 
    ofstream target("./keytrans.h");



    do {
        int xcode;
        int number;
        xToNumber >> xcode >> number;
        if (xToNumber.fail()) break;
        //cout << xcode << "-" << number << endl;
        target << "xToNumber[" << xcode << "]="<< number<< ";" << endl;
    }
    while (!xToNumber.eof());

    //cout << endl << endl;

    do {
        int number;
        string mode1;
        string mode2;
        string mode3;

        keynumberToScancode >> number >> mode1>>mode2>>mode3;
        if (keynumberToScancode.fail()) break;

        //unpack the multi char strings "0a,0b,0c" 
        //and add 0x before each hex value
        //unused values are set to 0xffff

        outNumberToScancode(target, 1, number, mode1);
        outNumberToScancode(target, 2, number, mode2);
        outNumberToScancode(target, 3, number, mode3);

        //cout << number << "." << mode1 << "." << mode2 << "." << mode3 << endl;
    } while (!keynumberToScancode.eof());




    return 0;
}

