в чем разница
int int32_t int_fast32_t int_least32_t

https://stackoverflow.com/questions/30339625/fastest-smallest-signed-integer-type
I am reading about the Fixed width integer types (cpp reference ) and come across the types int_fast8_t, int_fast16_t, int_fast32_t and int_least8_t, int_least16_t, int_least32_t,etc. My questions are following

What does it mean by saying for example int_fast32_t is fastest signed integer type (with at least 32 bits )? Is the more common type unsigned int slow?
What does it mean by saying for example int_least32_t is smallest signed integer type?
what are the differences between int_fast32_t, int_least32_t and unsigned int?

answer
int_fast32_t means that it is the fastest type of at least 32 bit for the processor. For most processors it is probably a 32 bit int. 
But imagine a 48 bit processor without a 32 bit add instruction. Keeping everything 48 bits is faster. int_least32_t is the smallest type for the target that can hold 32 bits. 
On the hypothetical 48 bit processor, there might be a 32 bit data type supported, with library support to implement them. Or int_least32_t might also be 48 bits. 
int is usually the fastest integer type for the target, but there is no guarantee as to the number of bits you'll get.