/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

extern CirMgr *cirMgr;

// TODO: Implement memeber functions for class(es) in cirGate.h

/**************************************/
/*   class CirGate member functions   */
/**************************************/
unsigned CirGate::_globalref = 1;
unsigned CirGate::_printcount = 0;
unsigned CirGate::_spaces = 0;

void
CirGate::reportGate() const
{
  string temp = getTypeStr();
  temp += '(';
  temp += to_string(_id);
  temp += ')';
  if(_name.size() > 0){
    temp += '\"';
    temp += _name;
    temp += '\"';
  }
  temp += ", line ";
  temp += to_string(_lineNo);
  cout << "==================================================" << endl;
  cout << "= " << left << setw(47)  << temp << '=' << endl;
  cout << "==================================================" << endl;
}

void
CirGate::reportFanin(int level) const
{
   assert (level >= 0);
   cout << getTypeStr() << ' ' << _id;
   if(_type == AIG_GATE || _type == PO_GATE){
     if(*_ref == _globalref && level > 0)                   cout << " (*)";
     cout << endl;
     if(level > 0 && *_ref != _globalref){                           
      *_ref = _globalref;
      ++_spaces;
      size_t pos = 0;
      while(pos < _faninList.size()){
        size_t i = 0;
        while(i < _spaces){
          ++i;
          cout << "  ";
        }
        if(_faninList[pos]->isInv())                        cout << '!';
        _faninList[pos]->gate()->reportFanin(level - 1);
        ++pos;
      }
      --_spaces;
     }
   }
   else                                                     cout << endl;
   if(_spaces == 0)                                         setGlobalref();
}

void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
   cout << getTypeStr() << ' ' << _id;
   if(_fanoutList.size() > 0){
     if(*_ref == _globalref && level > 0)                   cout << " (*)";
     cout << endl;
     if(*_ref != _globalref && level > 0){
       *_ref = _globalref;
       ++_spaces;
       size_t pos = 0;
       while(pos < _fanoutList.size()){
         size_t i = 0;
         while(i < _spaces){
           ++i;
           cout << "  ";
         }
         if(_fanoutList[pos]->isInv())                        cout << '!';
         _fanoutList[pos]->gate()->reportFanout(level - 1);
         ++pos;
       }
       --_spaces;
     }
   }
   else                                                     cout << endl;
   if(_spaces == 0)                                         setGlobalref();
}

void
PIGate::printGate() const{
  if(*_ref != _globalref){
    *_ref = _globalref;
    cout << '[' << _printcount << "] ";
    if(_type == PI_GATE){
      cout << "PI  " << _id;
      if(_name.size() != 0){
        cout << " (" << _name << ")";
      }
      cout << endl;
    }
    else{
      cout << "CONST0" << endl;
    }
    ++_printcount;
  }
}

void
AIGGate::printGate() const{
  if(*_ref != _globalref){
    *_ref = _globalref;
    bool inv[2] = {0};
    bool undef[2] = {0};
    if(_faninList[0]->gate()->getTypeStr() != "UNDEF")   _faninList[0]->gate()->printGate();
    else                                                 undef[0] = 1; 
    if(_faninList[0]->isInv() == 1)                      inv[0] = 1;
    if(_faninList[1]->gate()->getTypeStr() != "UNDEF")   _faninList[1]->gate()->printGate();
    else                                                 undef[1] = 1; 
    if(_faninList[1]->isInv() == 1)                      inv[1] = 1;
    cout << '[' << _printcount << "] ";
    cout << "AIG " << _id << ' ';
    if(undef[0] == 1)                                    cout << '*';
    if(inv[0]   == 1)                                    cout << '!';
    cout << _faninList[0]->gate()->getID() << ' ';
    if(undef[1] == 1)                                    cout << '*';
    if(inv[1]   == 1)                                    cout << '!';
    cout << _faninList[1]->gate()->getID();
    cout << endl;
    ++_printcount;
  }
}

void
POGate::printGate() const{
  if(*_ref != _globalref){
    *_ref = _globalref;
    bool inv = 0;
    bool undef = 0;
    if(_faninList[0]->gate()->getTypeStr() != "UNDEF")   _faninList[0]->gate()->printGate();
    else                                                 undef = 1; 
    if(_faninList[0]->isInv() == 1)                      inv = 1;
    cout << '[' << _printcount << "] ";
    cout << "PO  " << _id << ' ';
    if(undef == 1)                                       cout << '*';
    if(inv   == 1)                                       cout << '!';
    cout << _faninList[0]->gate()->getID();
    if(_name.size() != 0){
      cout << " (" << _name << ")";
    }
    cout << endl;
    ++_printcount;
  }
}

void
CirGate::writeAIG(ostream& outfile) const{
  size_t i = 0;
  while(i < _faninList.size()){
    if(_faninList[i]->gate()->_type == AIG_GATE && *_ref != _globalref)      _faninList[i]->gate()->writeAIG(outfile);
    ++i;
  }
  if(*_ref != _globalref && _type == AIG_GATE){
    outfile << _id*2 << ' ' << _faninList[0]->gate()->getID()*2 + _faninList[0]->isInv() 
            << ' ' << _faninList[1]->gate()->getID()*2 + _faninList[1]->isInv() << endl;
    *_ref = _globalref;
  }
}

void
CirGate::dfs() const {
  size_t i = 0;
  if(*_ref != _globalref){
    while(i < _faninList.size()){
      _faninList[i]->gate()->dfs();
      ++i;
    }
    *_ref = _globalref;
  }
}

