#ifndef EXTERNALTYPE
#define EXTERNALTYPE
#include <string>

using namespace std;

class ExternalType {
    public:
        virtual void SetNewValueFromUi(const string &)=0;
};
#endif
