#include "tfilter.h"

#include "textout.h"
#include <string.h>

/*
 * This is the general output engine for text.  It allows the various commands
 * to be stored as strings, with substitions from the arguments handled
 * as necessary.  In addition, it allows two models of font handling:
 * grouped (e.g., {\tt ...} or <TT> ... </TT>) and ungrouped (e.g., \n.I ...).
 * It also handles newline interpretation, ensuring that the "correct"
 * number of newlines is generated.
 * 
 * A command consists of a string with some special characters.  These are:
 * %% just a %
 * %1 Insert string s1
 * %2 Insert string s2 etc 
 * %i Insert integer i1 (but see note on definitions)
 * %n Insert newline ONLY if not AT a newline.
 * %N=".." Replace newlines with this string (%N revert to original behavior)
 *         Max of 32 characters.
 * %p Insert end_par ONLY if not already present.  Requires end_par to
 *    be defined.
 * %f Insert end-font-string, if any (not an error if none defined)
 * %e="..." Define end-font-string.  Max of 32 characters
 * %u1, %u2, etc.  Just like %1, but upper case the string.
 * %r1, %r2, etc.  The values of registers defined with %a or SetOutRegister
 * %rq1, etc. is like %r1, but the output is quoted (so that filters further 
 * downstream do not try to process the data)
 * %a1="..." define register 1.  Ditto for a2, etc.
 * %m="..." Define mode.  This is used for text systems that have special
 * processing modes, such as verbatim in LaTeX, where character processing
 * is changed.
 *
 * Others as needed
 * (We might want general registers to be filled or emptied, or some
 * way to indicated "paragraph only if you need to").  We need to be careful;
 * we could re-invent PostScript/Nroff/TeX/HTML/Fortran :).
 *
 * Strings out written with self if possible (see OutString)
 */

/* This the structure that holds the information about a command -
   Not used for now */
typedef struct { 
    char *cmdstring;
    } CmdInfo;

/* Names are matched as 
   cmdname-i1 (if i1 != TEXT_I_IGNORE)
      then
   cmdname
 */
int TextOut::MatchCommand( const char *cmdname, int i1, SrEntry **entry )
{
    if (i1 != TEXT_I_IGNORE) {
	char cmdbuf[256];
	sprintf( cmdbuf, "%s-%d", cmdname, i1 );
	if (!userops->Lookup( cmdbuf, entry )) return 0;
	return userops->Lookup( cmdname, entry );
	}
    else {
	return userops->Lookup( cmdname, entry );
	}
}

int TextOut::HasOp( const char *cmdname )
{
    SrEntry *entry;

    PutChar( 0 );
    if (!userops && next) 
        return next->HasOp( cmdname );
    return MatchCommand( cmdname, TEXT_I_IGNORE, &entry );
}

// 
// All of the PutOp commands use PutChar( 0 ) to flush any local buffers
//
int TextOut::PutOp( const char *cmdname, char *s1, char *s2, int i1 )
{
    SrEntry *entry;

    PutChar( 0 );
    if (!userops && next) 
        return next->PutOp( cmdname, s1, s2, i1 );
    if (MatchCommand( cmdname, i1, &entry )) {
	/* Did not find, so do nothing */
	return PutOpError( cmdname, (char *)0 );
	}
    else {
	PutCommand( (char *)entry->extra_data, s1, s2, (char *)0, (char *)0, 
		    i1 );
	}
    return 0;
}

int TextOut::PutOp( const char *cmdname, 
		    char *s1, char *s2, char *s3, char *s4 )
{
    SrEntry *entry;

    PutChar( 0 );
    if (!userops && next) 
        return next->PutOp( cmdname, s1, s2, s3, s4 );
    if (userops->Lookup( cmdname, &entry )) {
	/* Did not find, so do nothing */
	return PutOpError( cmdname, (char *)0 );
	}
    else {
	PutCommand( (char *)entry->extra_data, s1, s2, s3, s4, 0 );
	}
    return 0;
}

int TextOut::PutOp( const char *cmdname, char *s1 )
{
    SrEntry *entry;

    PutChar( 0 );
    if (!userops && next) 
        return next->PutOp( cmdname, s1 );
    if (userops->Lookup( cmdname, &entry )) {
	/* Did not find, so do nothing */
	return PutOpError( cmdname, (char *)0 );
	}
    else {
	PutCommand( (char *)entry->extra_data, s1, (char *)0, (char *)0, 
		    (char *)0, 0 );
	}
    return 0;
}

int TextOut::PutOp( const char *cmdname )
{
    SrEntry *entry;

    PutChar( 0 );
    if (!userops && next) 
        return next->PutOp( cmdname );
    if (userops->Lookup( cmdname, &entry )) {
	/* Did not find, so do nothing */
	return PutOpError( cmdname, (char *)0 );
	}
    else {
	PutCommand( (char *)entry->extra_data, (char *)0, (char *)0, 
		    (char *)0, (char *)0, 0 );
	}
    return 0;
}

int TextOut::PutOp( const char *cmdname, char *s1, int i1 )
{
    SrEntry *entry;

    PutChar( 0 );
    if (!userops && next) 
        return next->PutOp( cmdname, s1, i1 );
    if (MatchCommand( cmdname, i1, &entry )) {
	/* Did not find, so do nothing */
	return PutOpError( cmdname, (char *)0 );
	}
    else {
	PutCommand( (char *)entry->extra_data, s1, (char *)0, (char *)0, 
		    (char *)0, i1 );
	}
    return 0;
}

/* Default error behavior for PutOp command not found.
 */
int TextOut::PutOpError( const char *cmdname, const char *msg )
{
	if (msg) fprintf( stderr, "%s\n", msg);
	else 
		fprintf( stderr, "Did not find definition for command %s\n", cmdname );
	return 1;
}

int TextOut::SetDebug( int flag )
{
    debug_flag = flag;
    return 0;
}

int TextOut::SetOutstream( OutStream *in_outs )
{
    out = in_outs;
    if (debug_flag) out->Debug(debug_flag);
    if (next) next->SetOutstream( in_outs );
    return 0;
}

int TextOut::Flush( )
{
    return 0;
}
TextOut::~TextOut()
{
}

#include <ctype.h>

char * TextOut::GetDQtext( char *cmdstring, char *out_str, int maxlen )
{
    /* Skip the =, if any */
    *out_str = 0;
    if (*cmdstring == '=') cmdstring++;
    if (*cmdstring++ != '"') return 0;
    while (maxlen && *cmdstring != '"') {
	*out_str++ = *cmdstring++;
	maxlen--;
	}
    *out_str = 0;
    if (*cmdstring == '"') cmdstring++;
    return cmdstring;
}

/*
 * Since we want to allow optional processing of new lines, we have a special
 * output routine
 */

int TextOut::OutString( const char *string )
{
    char ch;
    int  rc = 0;

    if (!string) return 0;
    if (next) {
      if (debug_flag) printf( "TextOut::Outstring to next TextOut\n" );
      return next->OutString( string );
    }
    
    if (debug_flag)
      printf( "TextOut::Outstring to related Outstream %s\n", out->MyName() );
#ifdef FOO
    if (!nl) 
	rc = out->PutToken( 0, string );
    else {
	while (!rc && (ch = *string++)) {
	    if (ch == '\n') 
		rc = out->PutQuoted( 0, nl );
	    else 
		rc = out->PutChar( ch );
	    }
	}
#else
    if (!nl) 
	rc = PutToken( 0, string );
    else {
	while (!rc && (ch = *string++)) {
	    if (ch == '\n') 
		rc = PutQuoted( 0, nl );
	    else 
		rc = PutChar( ch );
	    }
	}
#endif
	if (rc) return err->ErrMsg( rc, "Error writing character" );
	else   return 0;
}

/* Upper-case version of OutString */
int TextOut::OutStringUC( const char *string )
{
    char ch;
    int  rc = 0;

    if (next)
    	return next->OutStringUC( string );
    if (!nl) {
      while (*string) {
	rc = out->PutChar( toupper( *string ) ); 
	string++;
      }
    }
    else {
	while (!rc && (ch = *string++)) {
	    if (ch == '\n') 
		rc = out->PutQuoted( 0, nl );
	    else 
		rc = out->PutChar( toupper( ch ) );
	    }
	}
	if (rc) return err->ErrMsg( rc, "Error writing character" );
	else   return 0;
}

/* 
 * This COULD use varargs or arg matching
 */
int TextOut::PutCommand( char *cmdstring, 
			 char *s1, char *s2, char *s3, char *s4, int i1 )
{
    char ch, ch2;
    int  l_was_par;
    int  l_was_nl;

    if (!cmdstring) return 1;

    if (debug_flag) 
	printf( "Processing command string %s\n", cmdstring );
    /* Interpret and replace string */
    while ((ch = *cmdstring++)) {
	if (ch == '%') {
	    ch2 = *cmdstring++;
	    if (debug_flag) printf( "Processing command %%%c\n", ch2 );
	    l_was_par	 = last_was_par;
	    last_was_par = 0;
	    l_was_nl     = last_was_nl;
	    last_was_nl  = 0;
	    switch (ch2) {
	        int regnum, r_quoted;
		case '%': out->PutChar( ch ); break;
	        case '1': OutString( s1 ); break;
                case '2': OutString( s2 ); break;
                case '3': OutString( s3 ); break;
                case '4': OutString( s4 ); break;
  	        case 'a': 
		  /* Get the register number and string value */
		  ch2 = *cmdstring++;
		  regnum = ch2 - '0';
		  if (regnum < 0 || regnum >= MAX_TEXT_REGNUM) return 1;
		  cmdstring = GetDQtext( cmdstring, cmdreg[regnum], 
					 MAX_TEXT_REGISTER );
		  //printf( "stng=%s\n", cmdreg[regnum] );
		  /* if cmdstring == 0, an error was found */
		  if (!cmdstring) return 1;
		  
		  break;
	        case 'r':
	          /* Get the register number and output the value in that
		     register */
		  ch2 = *cmdstring++;
		  r_quoted = (ch2 == 'q');
		  if (r_quoted) ch2 = *cmdstring++;
		  //if (debug_flag & r_quoted) 
		  //  printf( "quoted output\n" );
		  regnum = ch2 - '0';
		  if (regnum < 0 || regnum >= MAX_TEXT_REGNUM) return 1;
		  //printf( "stng_out%d=%s\n", regnum, cmdreg[regnum] );
		  if (r_quoted) 
		    out->PutQuoted( 0, cmdreg[regnum] );
		  else
		    out->PutToken( 0, cmdreg[regnum] );
		  break;

   	        case 'e': 
		  /* Get replacement string; (="...") set lfont */
		  cmdstring = GetDQtext( cmdstring, font_str, 32 );
		  /* if cmdstring == 0, an error was found */
		  if (!cmdstring) return 1;
		  lfont = font_str;
		  break;
		case 'f': if (lfont) { out->PutToken( 0, lfont ); }
			  lfont = 0;
			  break;
	        case 'i': out->PutInt( i1 ); break;
   	        case 'm': 
		  /* Get mode string; (="...") set mode */
		  cmdstring = GetDQtext( cmdstring, mode_str, 32 );
		  /* if cmdstring == 0, an error was found */
		  if (!cmdstring) return 1;
		  // An empty mode_str give a null mode (simplifies binary 
		  // modes)
		  if (*mode_str) 
		    mode = mode_str;
		  else 
		    mode = 0;
		  break;
	        case 'n': if (!l_was_nl) {
		             out->PutToken( 0, newline_onoutput );
			     UpdateNL( 1 );
		             }
		    break;
 	        case 'N': 
		    if (*cmdstring == '=') {
			cmdstring = GetDQtext( cmdstring, newline_str, 32 );
			if (!cmdstring) return 1;
			nl = newline_str;
			}
		    else 
			nl = 0;
		    break;
		    
		case 'p': 
			  if (l_was_par == 0) 
			      PutOp( "end_par" );
			  last_was_par = 1;
			  break;
	        case 'u':
	                  ch2 = *cmdstring++;
			  switch (ch2) {
			  case '1': OutStringUC( s1 ); break;
			  case '2': OutStringUC( s2 ); break;
			  case '3': OutStringUC( s3 ); break;
			  case '4': OutStringUC( s4 ); break;
			  }
		          break;
		default:  out->PutChar( '%' );
			  out->PutChar( ch2 );
			  break;
		}
	    }
	else {
	    if (nl && ch == '\n') {
		out->PutToken( 0, nl );
		} 
	    else {
		out->PutChar( ch );
		}
	    if (ch) 
	      UpdateNL( ch == '\n' );
	    last_was_par = 0;
	    }
	}	
    return 0;
}

/* These are the default versions.  */
int TextOut::PutChar( const char ch )
{
  if (debug_flag && ch) printf( "Generic TextOut::PutChar of %c\n", ch );
    /* This really needs to remember newlines in last_was_nl */
    if (ch) 
      UpdateNL( ch == '\n' );
    if (next) return next->PutChar( ch );
    else      return out->PutChar( ch );
}

int TextOut::PutToken( int nsp, const char *token )
{
    /* This really needs to remember newlines in last_was_nl */
    if (token && token[0])
      UpdateNL( token[strlen(token)-1] == '\n' );
    if (next) return next->PutToken( nsp, token );
    else      return out->PutToken( nsp, token );
}

int TextOut::PutQuoted( int nsp, const char *token )
{
  if (debug_flag && token && *token) 
    printf( "Generic puttoken for %s\n", token );
  if (token && *token) UpdateNL( 0 );
  if (next) return next->PutQuoted( nsp, token );
  else      return out->PutQuoted( nsp, token );
}

int TextOut::PutNewline( )
{
  UpdateNL( 1 );
  if (next) return next->PutNewline();
  else      {
    char *p = newline_onoutput;
    int rc;
    while (*p) if ((rc = out->PutChar( *p++ ))) break;
    return rc;
  }
}
void TextOut::SetNewlineString( const char *str )
{
  strncpy( newline_onoutput, str, 3 );
  newline_onoutput[2] = 0;
}

const char *TextOut::SetMode( const char * str )
{
  const char * savemode = mode;
  mode = str;
  // printf( "Setting mode to %s\n", str ? str : "NULL" );
  return (const char *)savemode;
}

int TextOut::SetRegisterValue( int regnum, const char * val )
{
  //printf( "reg val %d is %s\n", regnum, val );
  if (strlen(val) >= MAX_TEXT_REGISTER) return 1;
  strncpy( cmdreg[regnum], val, MAX_TEXT_REGISTER-1 );
  return 0;
}

//
// This is used to propagate newline changes down the list
// Do we need up as well?
//
int TextOut::UpdateNL( int in_last_was_nl )
{
    if (next) next->UpdateNL( in_last_was_nl );
    last_was_nl = in_last_was_nl;
    return 0;
}

/*
   The format of the command file is
   command-name command-string
   # is used for comments, and \ is used (at the end of the line ONLY) to
   continue to the next line.

   Note that the user can define ANY commands; they are added to the table.
   
   Note that command-names can have the form
   foo-3
   This will match a command name of foo and an integer value of 3.  This
   is a simple way to specify different behavior for different values
   of an input integer (e.g., heading level or itemize depth).
 */
#define MAX_NAME 128
#define MAX_COMMAND 1024
int TextOut::ReadCommands( InStream *ins ){
    char    ch;
    char    nametoken[MAX_NAME], *namep;
    char    *p, command[MAX_COMMAND], *pold;
    int     ln, nsp, maxlen;
    SrEntry *entry;
    int     prepend, postpend;   // indicate whether the command is added
    				 // or replaces existing command.

    ins->SetBreakChar( '-', BREAK_ALPHA );
    ins->SetBreakChar( '_', BREAK_ALPHA );
    ins->SetBreakChar( '+', BREAK_ALPHA );
    while (!ins->GetToken( MAX_NAME, nametoken, &nsp )) {
	/* Skip comments */
	if (nametoken[0] == '#') {
	    ins->SkipLine();
	    continue;
	    }
	// Check for prepend/postpend markers
	prepend  = 0;
	postpend = 0;
	namep    = nametoken;
	if (namep[0] == '+') {
	    prepend = 1;
	    namep++;
	    }
	ln = strlen( nametoken );
	if (nametoken[ln-1] == '+') {
	    postpend = 1;
	    nametoken[ln-1] = 0;
	    }
	if (prepend && postpend) {
	    // Should issue error message
	    postpend = 0;
	    }
	   
	/* Skip the leading space (upto newline) */
	while (!ins->GetChar( &ch ) && isspace( ch ) && ch != '\n') ;
        ins->UngetChar( ch );
        
	// Get command string for name 
	p      = command;
	maxlen = MAX_COMMAND - 1;
	// Allow \<space> and \<newline> as special characters.
	while (!ins->GetChar( &ch ) && maxlen > 0) {
	    if (ch == '\n') break;
	    if (ch == '\\') {
		ins->GetChar( &ch );
		if (ch == '\n') 
		    continue;
		else if (ch != ' ') {
		    *p++ = '\\';
		    maxlen--;
		    }
		}
	    *p++ = ch;
	    maxlen--;
	    }
	if (ch != '\n' && maxlen == 0) {
	  // command too long
	  return 1;
	}
	*p = 0;
	// Add to definitions.
	// namep is nametoken, with optional + stripped off.
	if (!userops->Insert( namep, &entry )) {
	    /* Install the definition of this command in the table */
	    /* Handle the case of prepending or postpending a definition */
	    if (entry->extra_data) {
	    	pold = (char *)entry->extra_data;
	        if (prepend) {
	            p = new char[strlen(command) + strlen(pold) + 1];
	            if (!p) return 1;
	            strcpy( p, command );
	            strcat( p, pold );
	            }
	        else if (postpend) {
	            p = new char[strlen(command) + strlen(pold) + 1];
	            if (!p) return 1;
	            strcpy( p, pold );
	            strcat( p, command );
	            }
	        else {
	            p = new char[strlen(command)+1];
	            if (!p) return 1;
	            strcpy( p, command );
	            }
	        delete (char*)entry->extra_data;
	        }
	    else {
	            p = new char[strlen(command)+1];
	            if (!p) return 1;
	            strcpy( p, command );
	        }
	    entry->extra_data = (void *)p;
	    }
        }
    return 0;
}

int TextOut::Debug( int flag )
{
  int old_flag = debug_flag;
  debug_flag = flag;
  return old_flag;
}

/* 
   It is sometimes useful to define an output stream that is really a textout
   object.  
 */
TextOutStrm::TextOutStrm( OutStream * outs )
{
    out		 = outs;
    next	 = 0;

    lfont	 = 0;
    nl		 = 0;
    last_was_nl	 = 0;
    last_was_par = 0;
    debug_flag	 = 0;
    userops	 = 0;
    strcpy( newline_onoutput, "\n" );
}
TextOutStrm::~TextOutStrm()
{
    if (out) delete out;
    out = 0;
}
