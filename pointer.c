/*
 * pointer.c - Source file with your solutions to the Lab.
 *             This is the file you will hand in to your instructor.
 */
#ifndef COMMON_H
#include "common.h"
#endif

/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

#if 0
You will provide your solution to this lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:

  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code
  must conform to the following style:

  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Function arguments and local variables (no global variables).
  2. Integer literals.

  Some of the problems restrict the set of allowed operators.

  You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions (unless otherwise stated).
  4. Use any operators not "allowed" at the top of the method you are writing.

  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has *undefined* behavior when shifting by a negative amount or an amount
     greater than or equal to the number of bits in the value being shifted.
     e.g. For x >> n, shifts by n < 0 or n >= *size of x* are undefined
     e.g. if x is a 32-bit int, shifts by >= 32 bits are undefined
     Undefined means you do not know what result you will get from the operation.
#endif

/*
 * STEP 2: Modify the following functions according the coding rules.
 */

/*
 * Return the size of an integer in bytes.
 *
 *  Calculate the size of an int using only arithmetic and casts!
 * DISALLOWED:
 *   Pointer operators: [] (Array Indexing Operator)
 *   Any non arithmetic operatror
 */
int intSize() {
  /*forces the int pointers to behave like char pointers and do 
  pointer arithmetic with a single byte instead of the size of an int
  This allows the two pointers to calculate the distance between them
  in bytes rather than the size of an int*/
  int intArray[2];
  int *intPtr1 = intArray;
  int *intPtr2 = intArray + 1;
  return (char*)intPtr2 - (char*)intPtr1;
}


/*
 * Swap the 2 ints.
 *
 * Write code that swaps the contents of the 2 ints
 *   Kudos if you do it without a temp variable 
 *   ( But you need to know your bitwises! :)
 *   
 */
void swapInts(int *i1, int *i2){
  //used a temporary variable to store the value of one int to switch
  int temp = *i1;
  *i1 = *i2;
  *i2 = temp;
}


/*
 * Write the 4 Bytes that compose the integer "value" (little-endian)
 *   into the char array "array". They should be written 
 *   in big-endian byte order.
 *   Assume the char* points at a memory location with 4B available
 * DISALLOWED:
 *   Pointer operators: [] (Array Indexing Operator)
 */
void serializeBE(unsigned char *array, int value) {
  /*shifts the value and the pointer to the array to store the correct bytes
   into the correct places
  */
    *(array+3) = value;
    *(array+2) = value>>8;
    *(array+1) = value>>16;
    *array = value>>24;
}

/*
 * Write the 4 Bytes in the char array "array" (big-endian byte order) 
 *   into the integer "value" in little-endian byte order
 *   Assume the char* points at a memory location with 4B available
 * DISALLOWED:
 *   Pointer operators: [] (Array Indexing Operator)
 */
void deserializeBE(int* value, const unsigned char *array) {
  /*stores each byte into the int then shifts that byte left so it can use bitwise
  or to put the next byte into the int
  */
  *value = *array;
  *value = *value<<8 | *(array+1);
  *value = *value<<8 | *(array+2);
  *value = *value<<8 | *(array+3);
}

/*
 * Return 1 if ptr points to an element within the specified intArray, 0 otherwise.
 * Pointing anywhere in the array is fair game, ptr does not have to
 * point to the beginning of an element. Check the spec for examples if you are 
 * confused about what this method is determining.
 * size is the number of elements of intArray. Assume size != 0.
 *
 * DISALLOWED:
 *   Pointer operators: [] (Array Indexing Operator)
 */
int withinArray(int *intArray, int size, int *ptr) {  
  /*using poitner arithmetic to calculate the different between the
  adressin the pointer and the address of the start of the array
  to find if the adress of the pointer is in the array or not
  */
  if ((ptr - intArray) >= 0 && (ptr-intArray) < size)
    return 1;
  return 0;
}

/*
 * In C characters are are terminated by the null character ('\0')
 * given a pointer to the start of the string return the length of this string.
 * (The null character is not counted as part of the string length.)
 *
 * DISALLOWED:
 *   Pointer operators: [] (Array Indexing Operator)
 */
int stringLength(char *s) {
  /*The while loop takes advantage of integers used as booleans and continues
  to loop until the null ternimator, incrementing a length variable as it goes
  */
  int length = 0;
  while (*s)
  {
    length++;
    s++;
  }
  return length;
}

/*
 * Returns the length of the initial portion of str1 which consists only of characters that are part of str2.
 * The search does not include the terminating null-characters of either strings, but ends there.
 * 
 * Example, stringSpan("abcdefgh", "abXXcdeZZh"); // returns 5
 *  
 * DISALLOWED:
 *   Pointer operators: [] (Array Indexing Operator)
 */
int stringSpan(char * str1, char * str2) {
  /* walks across both strings: if the characters are equal, increases length and 
  advances the first string's pointer, and if the characters are not equal it 
  advances the second string's poitner
  */
  int length = 0;
  while(*str2)
  {
    if(*str1 == *str2)
    {
      str1++;
      length++;
    }
    else str2++;
  }
  return length;
}

/*
 * Selection sort is a sorting algorithim that works by partitioning the array into
 * a sorted section and unsorted section. Then it repeatedly selects the minimum element
 * from the unsorted section and moves it to the end of the sorted section.
 *
 * So the pseudo-code might look something like this:
 * arr - an array
 * n - the length of arr
 *
 * for i = 0 to n - 1
 *   minIndex = i
 *   for  j = i + 1 to n
 *       if arr[minIndex] > arr[j]
 *           minIndex = j
 *       end if
 *   end for
 *   Swap(arr[i], arr[minIndex])
 * end for
 *
 * Implement selection sort below, it might be helpful to use the swapInts function you
 * defined earlier.
 *
 * DISALLOWED:
 *   Pointer operators: [] (Array Indexing Operator)
 */
void selectionSort(int *array, int arrLength) {
  /* walks the array one element at a time to see if the elements are in sorted order, 
  and swaps them if not
  */
  int minIndex = 0;
  for (int i=0; i<=arrLength-1; i++)
  {
    minIndex = i;
    for(int j=i+1; j<=arrLength-1; j++)
    {
      if (*(array+minIndex) > *(array+j))
        minIndex = j;
    }

    int temp = *(array+i);
    *(array+i) = *(array+minIndex);
    *(array+minIndex) = temp;
  }

}
