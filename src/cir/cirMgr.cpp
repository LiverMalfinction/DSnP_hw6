/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine constant (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
  clear();
  colNo = 0;
  lineNo = 0;
  ifstream ifs(fileName);
  if(ifs.is_open()){
    string head;
    string aag = "aag";
    //parseHead
    //cerr << head << "fuck" << endl;
    getline(ifs,head);
    //cerr << head << "fuck" << endl;
    if(head.size() == 0)                { errMsg = "aag"; return parseError(MISSING_IDENTIFIER); }
    else if(head[0] == ' ')             return parseError(EXTRA_SPACE);
    else if(head[0] == 9)               { errInt = 9; return parseError(ILLEGAL_WSPACE); }
    size_t subpos = 0;
    size_t token_num = 0;
    int arg[5] = {0};
    string str;
    lineNo = 0;
    for(colNo = 0; colNo < head.size(); ++colNo){
      //cerr << colNo << " " << token_num << " ";
      if(head[colNo] == ' '){
        if(head[colNo + 1] == ' ')           { ++colNo; return parseError(EXTRA_SPACE); }
        else if(head[colNo + 1] == '\t')     { ++colNo; errInt = 9; return parseError(ILLEGAL_WSPACE); }
        else{
          ++token_num;
          str.append(head, subpos, colNo - subpos);
          //cerr << "str is " << str << endl;
          subpos = colNo + 1;
          if(token_num == 1){
            if(str != aag) { 
              //cerr << str[0] << ' ' << str[1] << ' ' << str[2] << ' ' << (int)str[3] << endl;
              if(str.size() > 3 && str[0] == 'a' && str[1] == 'a' && str[2] == 'g' && str[3] > 47 && str[3] < 58){
                colNo = 3; return parseError(MISSING_SPACE);             
              }
              else      { errMsg = str; return parseError(ILLEGAL_IDENTIFIER); }
            }
          }
          else{
            if(myStr2Int(str, arg[token_num - 2]) == 0 || arg[token_num - 2] < 0){
              if(token_num == 2)                  errMsg = "number of variables(";
              else if (token_num == 3)            errMsg = "number of PIs(";
              else if (token_num == 4)            errMsg = "number of latches(";
              else if (token_num == 5)            errMsg = "number of POs(";
              else if (token_num == 6)            errMsg = "number of AIGs(";
              errMsg.append(str,0,str.length());
              errMsg.append(1,')');
              return parseError(ILLEGAL_NUM);
              //cerr << "arg[" << token_num - 2 << "] is " << arg[token_num - 2] << endl;
            }
          }
          str = "";
        }
      }
      if(token_num >= 6){
        //cerr << token_num << " " << endl;
        return parseError(MISSING_NEWLINE);
      }
      else if(head[colNo] == '\t'){
        errInt = 9;
        return parseError(MISSING_SPACE);
      }
    }
    if(head[colNo - 1] != ' '){
      ++token_num;
      str.append(head, subpos, colNo - subpos);
      if(token_num == 1){
        if(str != aag){
          errMsg = str;
          return parseError(ILLEGAL_IDENTIFIER);
        }
      }
      else{
        if(myStr2Int(str, arg[token_num - 2]) == 0 || arg[token_num - 2] < 0){
          if(token_num == 2)                  errMsg = "number of variables(";
          else if (token_num == 3)            errMsg = "number of PIs(";
          else if (token_num == 4)            errMsg = "number of latches(";
          else if (token_num == 5)            errMsg = "number of POs(";
          else if (token_num == 6)            errMsg = "number of AIGs(";
          errMsg.append(str,0,str.length());
          errMsg.append(1,')');
          return parseError(ILLEGAL_NUM);
          //cerr << "arg[" << token_num - 2 << "] is " << arg[token_num - 2] << endl;
        }
      }
      str = "";
    }
    //cerr << token_num << endl;
    if(token_num == 1)                   { errMsg = "number of variables";  return parseError(MISSING_NUM); }
    else if(token_num == 2)              { errMsg = "number of PIs";        return parseError(MISSING_NUM); }
    else if(token_num == 3)              { errMsg = "number of latches";    return parseError(MISSING_NUM); }
    else if(token_num == 4)              { errMsg = "number of PIs";        return parseError(MISSING_NUM); }
    else if(token_num == 5)              { errMsg = "number of AIGs";       return parseError(MISSING_NUM); }
    //cerr << "M : " << arg[0] << " I : " << arg[1] << " L : " << arg[2] << " O : " << arg[3] << " A : " << arg[4] << endl;
    else if(arg[0] < arg[1] + arg[2] + arg[4]){
      errInt = arg[0];
      errMsg = "Number of variables";
      return parseError(NUM_TOO_SMALL);
    }
    if(token_num == 6 && arg[2] > 0)      { errMsg = "latches";              return parseError(ILLEGAL_NUM); }
    //push all the possible gate'ptr into the vector
    size_t num_of_ptr = 0;
    while(num_of_ptr <= arg[0] + arg[3]){
      _totalList.push_back(NULL);
      ++num_of_ptr;
    }
    _totalList[0] = new PIGate(0, CONST_GATE, 0);
    //cerr << "PI : " << _PIList.size() << " PO : " << _POList.size() << " AIG : " << _AIGList.size() << " Total : " << _totalList.size() << endl;
    //parsing PI
    colNo = 0;
    ++lineNo;
    while(lineNo <= arg[1]){
      getline(ifs, head);
      if(head.size() == 0){                     
        if(ifs.eof())                           { clear(); errMsg = "PI"; return parseError(MISSING_DEF); }
        else                                    { clear(); errMsg = "PI literal ID"; return parseError(MISSING_NUM); }
      }
      while(colNo < head.size()){
        if(head[colNo] == ' ' && colNo == 0)    { clear(); return parseError(EXTRA_SPACE); }
        else if(head[colNo] == 9 && colNo == 0) { clear(); errInt = 9; return parseError(ILLEGAL_WSPACE); }
        else if(head[colNo] == ' ')             { clear(); return parseError(MISSING_NEWLINE); }
        ++colNo;
      }
      int lit;
      if(myStr2Int(head, lit) && lit >= 0 && lit <= arg[0]*2){
        int var = lit/2;
        if(lit == 1 || lit == 0)                { clear(); errInt = lit; colNo = 0; return parseError(REDEF_CONST); }
        else if(lit%2 == 1){
          clear();
          colNo = 0;
          errInt = lit;
          errMsg = "PI";
          return parseError(CANNOT_INVERTED);
        }
        else if(_totalList[var] == NULL){
          _totalList[var] = new PIGate(var, PI_GATE, lineNo + 1);
          _PIList.push_back(_totalList[var]);
        }
        else{
          errGate = _totalList[var];
          errInt = lit;
          clear();
          return parseError(REDEF_GATE);
        }
      }
      else if(lit > arg[0]*2){
        clear();
        errInt = lit;
        colNo = 0;
        return parseError(MAX_LIT_ID);
      }
      else{
        clear();
        errMsg = "PI literal ID(";
        errMsg.append(head);
        errMsg.append(1, ')');
        return parseError(ILLEGAL_NUM);
      }
      colNo = 0;
      ++lineNo;
    }
    //parsePO
    while(lineNo <= arg[1] + arg[3]){
      getline(ifs, head);
      if(head.size() == 0){                     
        if(ifs.eof())                           { clear(); errMsg = "PO"; return parseError(MISSING_DEF); }
        else                                    { clear(); errMsg = "PO literal ID"; return parseError(MISSING_NUM); }
      }
      while(colNo < head.size()){
        if(head[colNo] == ' ' && colNo == 0){
          clear();
          return parseError(EXTRA_SPACE);
        }
        else if(head[colNo] == 9 && colNo == 0){
          clear();
          errInt = 9;
          return parseError(ILLEGAL_WSPACE);
        }
        else if(head[colNo] == ' '){
          clear();
          return parseError(MISSING_NEWLINE);
        }
        ++colNo;
      }
      int lit = 0;
      if(myStr2Int(head, lit) && lit >= 0 && lit <= 2*arg[0] + 1){
        int var = lit/2;
        int inv = lit%2;
        _totalList[arg[0] + lineNo - arg[1]] = new POGate(arg[0] + lineNo - arg[1], PO_GATE, lineNo + 1);
        _POList.push_back(_totalList[arg[0] + lineNo - arg[1]]);
        if(_totalList[var] == NULL){
          _totalList[var] = new AIGGate(var, UNDEF_GATE, 0);
        }
        GateV* input = new GateV(_totalList[var], inv);
        GateV* output = new GateV(_totalList[arg[0] + lineNo - arg[1]], inv);
        _totalList[arg[0] + lineNo - arg[1]]->connectFanin(input);
        _totalList[var]->connectFanout(output);
        //_totalList[var]->testFanout(); 
      }
      else if(lit > arg[0]*2 + 1){
        clear();
        errInt = lit;
        colNo = 0;
        return parseError(MAX_LIT_ID);
      }
      else{
        clear();
        errMsg = "PO literal ID(";
        errMsg.append(head);
        errMsg.append(1, ')');
        return parseError(ILLEGAL_NUM);
      }
      colNo = 0;
      ++lineNo;
    }
    //parseAIG
    token_num = 0;
    subpos = 0;
    //cerr << arg[1] + arg[3] +arg[4] << endl;
    while(lineNo <= arg[1] + arg[3] +arg[4]){
      //cerr << lineNo << endl;
      int lit[3] = {0};
      getline(ifs, head);
      if(head.size() == 0){                     
        if(ifs.eof())                           { clear(); errMsg = "AIG"; return parseError(MISSING_DEF); }
        else                                    { clear(); errMsg = "AIG literal ID"; return parseError(MISSING_NUM); }
      }
      if(head[0] == ' ')                                    { clear(); return parseError(EXTRA_SPACE); }
      while(colNo < head.size()){
        if(head[colNo] == ' '){
          if(head[colNo + 1] == ' ')                        { clear();  ++colNo; return parseError(EXTRA_SPACE); }
          else if(head[colNo] == 9)                         { clear(); errInt = 9; return parseError(ILLEGAL_WSPACE); }
          else{
            ++token_num;
            str.append(head, subpos, colNo - subpos);
            //cerr << subpos << ' ' << colNo << endl;
            if(myStr2Int(str, lit[token_num - 1]) && lit[token_num - 1] >= 0){
              if(lit[token_num - 1] <= arg[0]*2 + 1){
                int var = lit[token_num - 1]/2;
                int inv = lit[token_num - 1]%2;
                if(var == 0 && token_num == 1)                        { clear(); colNo = 0; errInt = lit[0]; return parseError(REDEF_CONST);}
                  if(token_num == 1 && inv == 0){
                  if(_totalList[var] == NULL)                         _totalList[var] = new AIGGate(var, AIG_GATE, lineNo + 1);
                  else if(_totalList[var]->getTypeStr() == "UNDEF")   { _totalList[var]->changeType(AIG_GATE); _totalList[var]->_lineNo = lineNo + 1; }
                  else{
                    errGate = _totalList[var];
                    errInt = lit[token_num - 1];
                    clear();
                    return parseError(REDEF_GATE);
                  }
                  _AIGList.push_back(_totalList[var]);
                }
                else if(token_num == 1 && inv == 1){
                  clear();
                  errInt = lit[0];
                  errMsg = "AIG";
                  return parseError(CANNOT_INVERTED);
                }
                else if(token_num == 2){
                  if(_totalList[var] == NULL)                         _totalList[var] = new AIGGate(var, UNDEF_GATE, 0);
                  GateV* input = new GateV(_totalList[var], inv);
                  GateV* output = new GateV(_totalList[lit[0]/2], inv);
                  _totalList[lit[0]/2]->connectFanin(input);
                  _totalList[var]->connectFanout(output);
                }
                else if(token_num >= 3){
                  //cerr << token_num << " " << endl;
                  clear();
                  return parseError(MISSING_NEWLINE);
                }
              }
              else{
                clear();
                errInt = lit[token_num - 1];
                colNo = subpos;
                return parseError(MAX_LIT_ID);
              }
            }
            else{
              clear();
              errMsg = "AIG Literal (";
              errMsg.append(str);
              errMsg.append(1,')');
              return parseError(ILLEGAL_NUM);
            }
            subpos = colNo + 1;
            str = "";
          }
        }
        else if(head[colNo] == '\t'){
          errInt = 9;
          clear();
          return parseError(MISSING_SPACE);
        }
        ++colNo;
      }
      if(head[colNo - 1] != ' '){
        //cerr << "good" << endl;
        ++token_num;
        str.append(head, subpos, colNo - subpos);
        if(myStr2Int(str, lit[token_num - 1]) && lit[token_num - 1] >= 0){
          if(lit[token_num - 1] <= arg[0]*2 + 1){
            int var = lit[token_num - 1]/2;
            int inv = lit[token_num - 1]%2;
            if(token_num == 1 && inv == 0){
              if(_totalList[var] == NULL)                         _totalList[var] = new AIGGate(var, AIG_GATE, lineNo + 1);
              else if(_totalList[var]->getTypeStr() == "UNDEF")   _totalList[var]->changeType(AIG_GATE);
              else{
                errGate = _totalList[var];
                errInt = lit[token_num - 1];
                clear();
                return parseError(REDEF_GATE);
              }
              _AIGList.push_back(_totalList[var]);
            }
            else if(token_num == 1 && inv == 1){
              clear();
              errInt = lit[0];
              errMsg = "AIG";
              return parseError(CANNOT_INVERTED);
            }
            else if(token_num >= 2){
              if(_totalList[var] == NULL)                         _totalList[var] = new AIGGate(var, UNDEF_GATE, 0);
              GateV* input = new GateV(_totalList[var], inv);
              GateV* output = new GateV(_totalList[lit[0]/2], inv);
              _totalList[lit[0]/2]->connectFanin(input);
              _totalList[var]->connectFanout(output);
            }
          }
          else{
            clear();
            errInt = lit[token_num - 1];
            colNo = subpos;
            return parseError(MAX_LIT_ID);
          }
        }
        else{
          clear();
          errMsg = "AIG Literal (";
          errMsg.append(str);
          errMsg.append(1,')');
          return parseError(ILLEGAL_NUM);
        }
        str = "";
        subpos = colNo + 1;
      }
      if(token_num <= 2 && head[head.size() - 1] != ' ')         { clear(); return parseError(MISSING_SPACE); }
      else if (token_num <= 2 && head[head.size() - 1] == ' ')   { clear(); errMsg = "AIG input literal ID"; return parseError(MISSING_NUM); }
      token_num = 0;
      subpos = 0;
      colNo = 0;
      ++lineNo;
    }
    //parsing naming
    do{
      getline(ifs, head);
      //cerr << head << endl;
      if(ifs.eof())                         { break; }
      if(head.size() == 0)                  { clear(); errMsg = head[0]; return parseError(ILLEGAL_SYMBOL_TYPE); }
      if(head[0] == 'i' || head[0] == 'o'){
        colNo = 1;
        if(head.size() == 1)                { clear(); errMsg = "symbol index"; return parseError(MISSING_NUM); }
        else if(head[1] == ' ')             { clear(); return parseError(EXTRA_SPACE); }
        else if(head[1] == '\t')            { clear(); errInt = 9; return parseError(ILLEGAL_WSPACE); }
        while(colNo < head.size() && head[colNo] != ' ' && head[colNo] != '\t') { ++colNo; }
        int idx = 0;
        subpos = colNo + 1;
        str = "";
        str.append(head, 1, colNo - 1);
        if(head[colNo] == '\t')             { clear(); return parseError(MISSING_SPACE); }
        else if(myStr2Int(str, idx) && idx >= 0){
          if      (head[0] == 'i' && idx < _PIList.size()){
            str = "";
            if(colNo == head.size())        { clear(); errMsg = "symbolic name"; return parseError(MISSING_IDENTIFIER); }
            colNo++;
            while(colNo < head.size() && head[colNo] > 31 && head[colNo] < 127) { ++colNo; }
            if(colNo == head.size()){
              str.append(head, subpos, head.size() - subpos);
              if(str.size() == 0)           { clear(); errMsg = "symbolic name"; return parseError(MISSING_IDENTIFIER); }
              else if (_PIList[idx]->_name.size() != 0) { clear(); errMsg = 'i'; errInt = idx; return parseError(REDEF_SYMBOLIC_NAME); }
              _PIList[idx]->setname(str);
            }
            else                            { clear(); errInt = (int)head[colNo];return parseError(ILLEGAL_SYMBOL_NAME); }
          }
          else if (head[0] == 'o' && idx < _POList.size()){
            str = "";
            colNo++;
            while(colNo < head.size() && head[colNo] > 31 && head[colNo] < 127) { ++colNo; }
            if(colNo == head.size()){
              str.append(head, subpos, head.size() - subpos);
              if(str.size() == 0)           { clear(); errMsg = "symbolic name"; return parseError(MISSING_IDENTIFIER); }
              else if (_POList[idx]->_name.size() != 0) { clear(); errMsg = 'o'; errInt = idx; return parseError(REDEF_SYMBOLIC_NAME); }
              _POList[idx]->setname(str);
            }
            else                            { clear(); errInt = (int)head[colNo];return parseError(ILLEGAL_SYMBOL_NAME); }
          }
          else                              { clear(); errInt = idx; errMsg = "PI index"; return parseError(NUM_TOO_BIG); }
        }
        else                                { clear(); errMsg = "symbol index("; errMsg.append(str); errMsg.append(1,')');return parseError(ILLEGAL_NUM); }
      }
      else if(head[0] == ' ')               { clear(); return parseError(EXTRA_SPACE); }
      else if(head[0] == '\t')              { clear(); errInt = 9; return parseError(ILLEGAL_WSPACE); }
      else if(head[0] != 'c')               { clear(); errMsg = head[0]; return parseError(ILLEGAL_SYMBOL_TYPE); }
      else if(head[0] == 'c'){
        if(head[1] == ' ')                  { clear(); colNo = 1; return parseError(MISSING_NEWLINE); }
      }
      colNo = 0;
      ++lineNo;
    }while(head != "c");
  }
  else                                      { cerr << "Cannot open design \"" << fileName << "\"!!" << endl; return false; }
  /*for(size_t i = 0; i < _totalList.size(); i++){
    //cout << _totalList[i] << endl;
  }*/
  //cerr << "M : " << arg[0] << "I : " << arg[1] << ":L : " << arg[0] << "M : " << arg[0] << "M : " << arg[0] << endl;
  return true;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
  if(_totalList.size() > 0)
  cout << "\nCircuit Statistics" << endl
       << "==================" << endl
       << "  " << setiosflags( ios::left ) << setw(5) << "PI"
       << resetiosflags( ios::left ) << setw(9) << _PIList.size() << endl
       << "  " << setiosflags( ios::left ) << setw(5) << "PO"
       << resetiosflags( ios::left ) << setw(9) << _POList.size() << endl
       << "  " << setiosflags( ios::left ) << setw(5) << "AIG"
       << resetiosflags( ios::left ) << setw(9) << _AIGList.size() << endl
       << "------------------" << endl
       << "  " << setiosflags( ios::left ) << setw(5) << "Total"
       << resetiosflags( ios::left ) << setw(9) << _AIGList.size() + _POList.size() + _PIList.size() << endl;
}

void
CirMgr::printNetlist() const
{
  cout << endl;
  size_t i = 0;
  while(i < _POList.size()){
    _POList[i]->printGate();
    ++i;
  }
  CirGate::_printcount = 0;
  CirGate::setGlobalref();
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   size_t i = 0;
   while(i < _PIList.size()){
     cout << ' ' << _PIList[i]->_id;
     ++i;
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   size_t i = 0;
   while(i < _POList.size()){
     cout << ' ' << _POList[i]->_id;
     ++i;
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
  vector<size_t> _flo_gates;
  vector<size_t> _unused_gates;
  size_t idx = 0;
  while(idx < _totalList.size()){
    if(_totalList[idx] != NULL){
      size_t i = 0;
      if(_totalList[idx]->_fanoutList.size() == 0 && _totalList[idx]->_type != PO_GATE && _totalList[idx]->_type != CONST_GATE)  { _unused_gates.push_back(idx); }
      while(i < _totalList[idx]->_faninList.size()){
        if(_totalList[idx]->_faninList[i]->gate()->_type == UNDEF_GATE)                  { _flo_gates.push_back(idx); }
        ++i;
      }
    }
    ++idx;
  }
  if(_flo_gates.size() > 0){                           cout << "Gates with floating fanin(s):";
    idx = 0;
    while(idx < _flo_gates.size()){
      cout << ' ' << _flo_gates[idx];
      ++idx;
    }
    cout << endl;
  }
  if(_unused_gates.size() > 0){                        cout << "Gates defined but not used  :";
    idx = 0;
    while(idx < _unused_gates.size()){
      cout << ' ' << _unused_gates[idx];
      ++idx;
    }
    cout << endl;
  }
}

void
CirMgr::writeAag(ostream& outfile) const
{
  outfile << "aag " << _totalList.size() - _POList.size() - 1 << ' ' << _PIList.size() << " 0 "
          << _POList.size() << ' ';
  unsigned num_of_aig = 0;
  size_t i = 0;
  while(i < _POList.size()){
    _POList[i]->dfs();
    ++i;
  }
  i = 0;
  while(i < _AIGList.size()){
    if(*_AIGList[i]->_ref == _AIGList[i]->_globalref)    ++num_of_aig;
    ++i;
  }
  CirGate::setGlobalref();
  outfile << num_of_aig << endl;
  i = 0;
  while(i < _PIList.size()){
    outfile << _PIList[i]->_id*2 << endl;
    ++i;
  }
  i = 0;
  while(i < _POList.size()){
    outfile << _POList[i]->_faninList[0]->gate()->getID()*2 + _POList[i]->_faninList[0]->isInv() << endl;
    ++i;
  }
  i = 0;
  while(i < _POList.size()){
    if(_POList[i]->_faninList.size() > 0)_POList[i]->_faninList[0]->gate()->writeAIG(outfile);
    ++i;
  }
  CirGate::setGlobalref();
  i = 0;
  while(i < _PIList.size()){
    if(_PIList[i]->_name.size() > 0){
      cout << 'i' << i << ' ' << _PIList[i]->_name << endl;
    }
    i++;
  }
  i = 0;
  while(i < _POList.size()){
    if(_POList[i]->_name.size() > 0){
      cout << 'o' << i << ' ' << _POList[i]->_name << endl;
    }
    i++;
  }
  cout << 'c' << endl;
}

void
CirMgr::clear(){
  size_t i = 0;
  while(i < _PIList.size()){
    delete _PIList[_PIList.size() - 1];
    _PIList.pop_back();
  }
  i = 0;
  while(i < _POList.size()){
    delete _POList[_POList.size() - 1];
    _POList.pop_back();
  }
  i = 0;
  while(i < _AIGList.size()){
    delete _AIGList[_AIGList.size() - 1];
    _AIGList.pop_back();
  }
  i = 0;
  _totalList.clear();
  CirGate :: _globalref = 1;
}
