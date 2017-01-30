//  lint rules

//lint -e1066  Symbol declared as "C" conflicts with line ..., file ...

//  eternal erroneous lint errors in working system headers
// Error 10: Expecting '}'
// Error 96: Unmatched left brace for linkage specification on line 8, file \mingw\include\rpcdce.h
// Error 96: Unmatched left brace for linkage specification on line 12, file \mingw\include\rpc.h
//lint -e10  Expecting '}'
//lint -e96  Unmatched left brace for linkage specification on line 8, file \mingw\include\rpcdce.h

//lint -e525  Negative indentation from line ...

//lint -e730  Boolean argument to function that takes boolean argument

//lint -e1704 Constructor has private access specification
//lint -e1719 assignment operator for class has non-reference parameter
//lint -e1720 assignment operator for class has non-const parameter
//lint -e1722 assignment operator for class does not return a reference to class

//  this is simply *wrong* on C++ functions...
// lint -e1066  Symbol declared as "C" conflicts with line 18, file statbar.h, module ...

//lint -e1762  Member function could be made const
//lint -e1714  Member function not referenced

//lint -e534  Ignoring return value of function
//lint -e716  while(1) ... 

//lint -e818  Pointer parameter could be declared as pointing to const
//lint -e830  Location cited in prior message
//lint -e831  Reference cited in prior message
//lint -e834  Operator '-' followed by operator '-' is confusing.  Use parentheses.
//lint -e840  Use of nul character in a string literal
//lint -e845  The right argument to operator 'ptr+int' is certain to be 0
//lint -e843  Variable could be declared as const
//lint -e1776 Converting a string literal to char * is not const safe (initialization)

//lint -esym(1784, WinMain)

//lint -e755  global macro not referenced

//lint -e713  Loss of precision (arg. no. 3) (unsigned int to int)
//lint -e732  Loss of sign (initialization) (int to unsigned int)

