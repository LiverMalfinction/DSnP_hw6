/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"

using namespace std;

class CirGate;
class PIGate;
class POGate;
class AIGGate;
class GateV;
//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes

class GateV
{
#define NEG 0x1
public:
  GateV() : _gateV(0) {}
  GateV(CirGate* g, size_t phase) : _gateV(size_t(g) + phase) {}
  CirGate* gate() const { return (CirGate*)(_gateV & ~(size_t)NEG); }
  bool isInv() const { return _gateV & NEG; }

private:
  size_t _gateV;

};

class CirGate
{
  friend class CirMgr;
public:
   CirGate()
     : _id(0), _type(UNDEF_GATE), _lineNo(0){ _ref = new unsigned(0); }
   CirGate(unsigned i, GateType t, unsigned l)
     : _id(i), _type(t), _lineNo(l){ _ref = new unsigned(0); }
   virtual ~CirGate() { delete _ref; }

   // Basic access methods
   string getTypeStr() const { 
     switch(_type){
       case UNDEF_GATE :      return "UNDEF";
       case PI_GATE :         return "PI";
       case PO_GATE :         return "PO";
       case AIG_GATE:         return "AIG";
       case CONST_GATE:       return "CONST";
       default:
       break;
     } 
   }
   void changeType(GateType t) { _type = t; }
   unsigned getLineNo() const { return _lineNo; }
   unsigned getID()     const { return _id; }
   void connectFanin(GateV* v) { _faninList.push_back(v); }
   void connectFanout(GateV* v) { 
     _fanoutList.push_back(v); 
     size_t i = _fanoutList.size() - 1;
     while(i > 0 && _fanoutList[i - 1]->gate()->getID() > _fanoutList[i]->gate()->getID()){
       GateV* temp = _fanoutList[i - 1];
       _fanoutList[i - 1] = _fanoutList[i];
       _fanoutList[i] = temp;
       --i;
     }
   }
   static void setGlobalref() { ++_globalref; }
   /*void testFanin() const { 
     for(size_t i = 0; i < _faninList.size(); i++){ 
       cerr << "Gate " << _faninList[i]->gate()->getID() << " is " << _faninList[i]->gate()->getTypeStr() << "." << endl;
       cerr << "And it is " << _faninList[i]->isInv() << endl;
     } 
   }
   void testFanout() const {
     for(size_t i = 0; i < _fanoutList.size(); i++){ 
       cerr << "Gate " << _fanoutList[i]->getID() << " is " << _fanoutList[i]->getTypeStr() << "." << endl;
     } 
   }*/
   

   // Printing functions
   virtual void printGate() const = 0;
   void setname(const string& name) { _name = name; }
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;
   void writeAIG(ostream& outfile) const;
   void dfs() const;

private:

protected:
   vector<GateV*>    _faninList;
   vector<GateV*>    _fanoutList;
   string            _name;
   GateType          _type;
   unsigned*         _ref;
   unsigned          _id;
   unsigned          _lineNo;
   static unsigned   _spaces;
   static unsigned   _globalref;
   static unsigned   _printcount;
};

class PIGate : public CirGate{
public:
   PIGate() : CirGate() {}
   PIGate(unsigned i, GateType t, unsigned l) : CirGate(i, t, l) {}
   ~PIGate() {}
   virtual void printGate() const;
protected:
};

class POGate : public CirGate{
public:
   POGate() : CirGate() {}
   POGate(unsigned i, GateType t, unsigned l) : CirGate(i, t, l) {}
   ~POGate() {}
   virtual void printGate() const;
protected:
};

class AIGGate : public CirGate{
public:
   AIGGate() : CirGate() {}
   AIGGate(unsigned i, GateType t, unsigned l) : CirGate(i, t, l) {}
   ~AIGGate() {}
   virtual void printGate() const;
};

#endif // CIR_GATE_H
