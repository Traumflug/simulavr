/*
 *  $Id$
 */

#ifndef EXTERNALTYPE
#define EXTERNALTYPE
#include <string>

class ExternalType {
 public:
    virtual void SetNewValueFromUi(const std::string &)=0;
    virtual ~ExternalType() {}
};
#endif
