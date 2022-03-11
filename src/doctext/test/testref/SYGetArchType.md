# SYGetArchType
Return a standardized architecture type for the machine that is executing this routine.  This uses uname where possible, but may modify the name (for example, sun4 is returned for all sun4 types). 
## Synopsis
```
void SYGetArchType( str, slen )
char *str;
int  slen;
```

## Input Parameter
slen - length of string buffer

## Output Parameter

str 
: string area to contain architecture name.  Should be at least 
10 characters long.



## Notes
This is a long block of text that contains a number of lines
and has several sentances.  Such as this one, which is the
second of many.  Well, maybe not many.  Maybe four or
five sentances?
**Location:**<A HREF="../../../../tstpgm.c#SYGetArchType">tstpgm.c</A>
