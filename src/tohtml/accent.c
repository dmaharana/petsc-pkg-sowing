#include "sowing.h"
#include "tex.h"

/*
 * This handles the accent commands \", \`, etc.
 *
 * See http://www.hut.fi/~jkorpela/HTML3.2/latin1.html for the HTML 
 * maps.
 *
 * We begin by defining a mapping of (accent command)letter to HTML.
 * Each accent command will search for the matching.  To keep this
 * simple, these are simple tables.
 */

/* Umlaut \" TeX accent 7F*/
static char *(umlauts[128]);

/* Grave \` TeX accent 18 */
static char *(graves[128]);

/* Accute \' TeX accent 19 */
static char *(acutes[128]);

/* Tilde \~ TeX accent 7E */
static char *(tildes[128]);

/* Circs \. TeX accent 95? */
static char *(circs[128]);

static int Debug_accents = 1;

static void InitUmlauts( void );
static void InitGraves( void );
static void InitAcutes( void );
static void InitTildes( void );
static void InitCircs( void );

#ifndef STRDUP 
#define STRDUP strdup
#endif

void TXoutAccent( TeXEntry * );

void InitAccents( void )
{
    int i;

    /* First, setup NO replacements */
    for (i=0; i<128; i++) {
	umlauts[i] = 0;
	graves[i]  = 0;
	acutes[i] = 0;
	tildes[i]  = 0;
    }

    /* Define the defaults */
    InitUmlauts();
    InitGraves();
    InitAcutes();
    InitTildes();
    InitCircs();

    /* Eventually, provide a way to add new definitions */

    /* Add these commands to the known names */
    TXInsertName( TeXlist, "\"", TXoutAccent, 1, (void *)0 );
    TXInsertName( TeXlist, "~", TXoutAccent, 1, (void *)0 );
    TXInsertName( TeXlist, "`", TXoutAccent, 1, (void *)0 );
    TXInsertName( TeXlist, "\'", TXoutAccent, 1, (void *)0 );
}

void TXoutAccent( TeXEntry *e )
{
    char accent = e->name[0];
    int  letter;
    char letterarray[2];

    /* Get the letter to process */
    letter =  SCTxtGetChar( fpin[curfile] );
    if (letter == LbraceChar) {
	/* We have a problem */
	SCPushChar( (char) letter );
	return;
    }

    /* Try to generate the relevant html*/
    switch (accent) {
    case '\"':
	if (umlauts[letter]) { TeXoutcmd( fpout, umlauts[letter] ); return; }
	break;

    case '\'':
	if (acutes[letter]) { TeXoutcmd( fpout, acutes[letter] ); return; }
	break;

    case '~':
	if (tildes[letter]) { TeXoutcmd( fpout, tildes[letter] ); return; }
	break;

    case '.':
	if (circs[letter]) { TeXoutcmd( fpout, circs[letter] ); return; }
	break;
	
    case '`':
	if (graves[letter]) { TeXoutcmd( fpout, graves[letter] ); return; }
	break;

    default:
	break;
    }
    letterarray[0] = letter;
    letterarray[1] = 0;
    TeXoutstr( fpout, letterarray );
}

static void InitUmlauts( void )
{
    umlauts['A']  = STRDUP( "&#196;" ); /* &Auml;  capital A, dieresis or umlaut mark */
    umlauts['E']  = STRDUP( "&#203;" ); /* &Euml;  &#203;  capital E, dieresis or umlaut mark  */
    umlauts['I']  = STRDUP( "&#207;" ); /* &Iuml;  &#207;  capital I, dieresis or umlaut mark  */
    umlauts['O']  = STRDUP( "&#214;" ); /* &Ouml;  &#214;  capital O, dieresis or umlaut mark  */
    umlauts['U']  = STRDUP( "&#220;" ); /* &Uuml;  &#220;  capital U, dieresis or umlaut mark  */
    umlauts['a']  = STRDUP( "&#228;" ); /* &auml;  &#228;  small a, dieresis or umlaut mark    */
    umlauts['e']  = STRDUP( "&#235;" ); /* &euml;  &#235;  small e, dieresis or umlaut mark    */
    umlauts['i']  = STRDUP( "&#239;" ); /* &iuml;  &#239;  small i, dieresis or umlaut mark    */
    umlauts['o']  = STRDUP( "&#246;" ); /* &ouml;  &#246;  small o, dieresis or umlaut mark    */
    umlauts['u']  = STRDUP( "&#252;" ); /* &uuml;  &#252;  small u, dieresis or umlaut mark    */
    umlauts['y']  = STRDUP( "&#255;" ); /* &yuml;  &#255;  small y, dieresis or umlaut mark    */
}

static void InitGraves( void )
{
    graves['A'] = STRDUP( "&#192;" ); /* &Agrave;  &#192;  capital A, grave accent  */
    graves['E'] = STRDUP( "&#200;" ); /* &Egrave;  &#200;  capital E, grave accent  */
    graves['I'] = STRDUP( "&#204;" ); /* &Igrave;  &#204;  capital I, grave accent  */
    graves['O'] = STRDUP( "&#210;" ); /* &Ograve;  &#210;  capital O, grave accent  */
    graves['U'] = STRDUP( "&#217;" ); /* &Ugrave;  &#217;  capital U, grave accent  */
    graves['a'] = STRDUP( "&#224;" ); /* &agrave;  &#224;  small a, grave accent    */
    graves['e'] = STRDUP( "&#232;" ); /* &egrave;  &#232;  small e, grave accent    */
    graves['i'] = STRDUP( "&#236;" ); /* &igrave;  &#236;  small i, grave accent    */
    graves['o'] = STRDUP( "&#242;" ); /* &ograve;  &#242;  small o, grave accent    */
    graves['u'] = STRDUP( "&#249;" ); /* &ugrave;  &#249;  small u, grave accent    */
}

static void InitAcutes( void )
{
    acutes['A'] = STRDUP( "&#193;" ); /* &Aacute;  &#193;  capital A, acute accent   */
    acutes['E'] = STRDUP( "&#201;" ); /* &Eacute;  &#201;  capital E, acute accent   */
    acutes['I'] = STRDUP( "&#205;" ); /* &Iacute;  &#205;  capital I, acute accent   */
    acutes['O'] = STRDUP( "&#211;" ); /* &Oacute;  &#211;  capital O, acute accent   */
    acutes['U'] = STRDUP( "&#218;" ); /* &Uacute;  &#218;  capital U, acute accent   */
    acutes['Y'] = STRDUP( "&#221;" ); /* &Yacute;  &#221;  capital Y, acute accent   */
    acutes['a'] = STRDUP( "&#225;" ); /* &aacute;  &#225;  small a, acute accent     */
    acutes['e'] = STRDUP( "&#233;" ); /* &eacute;  &#233;  small e, acute accent     */
    acutes['i'] = STRDUP( "&#237;" ); /* &iacute;  &#237;  small i, acute accent     */
    acutes['o'] = STRDUP( "&#243;" ); /* &oacute;  &#243;  small o, acute accent     */
    acutes['u'] = STRDUP( "&#250;" ); /* &uacute;  &#250;  small u, acute accent     */
    acutes['y'] = STRDUP( "&#253;" ); /* &yacute;  &#253;  small y, acute accent     */
}

static void InitTildes( void )
{
    tildes['A'] = STRDUP( "&#195;" ); /* &Atilde;  &#195;  capital A, tilde   */
    tildes['N'] = STRDUP( "&#209;" ); /* &Ntilde;  &#209;  capital N, tilde   */
    tildes['O'] = STRDUP( "&#213;" ); /* &Otilde;  &#213;  capital O, tilde   */
    tildes['a'] = STRDUP( "&#227;" ); /* &atilde;  &#227;  small a, tilde     */
    tildes['n'] = STRDUP( "&#241;" ); /* &ntilde;  &#241;  small n, tilde     */
    tildes['o'] = STRDUP( "&#245;" ); /* &otilde;  &#245;  small o, tilde     */
}

static void InitCircs( void )
{
    circs['A'] = STRDUP( "&Acirc;" ); /*   &#194;  capital A, circumflex accent   */
    circs['E'] = STRDUP( "&Ecirc;" ); /*   &#202;  capital E, circumflex accent   */
    circs['I'] = STRDUP( "&Icirc;" ); /*   &#206;  capital I, circumflex accent   */
    circs['O'] = STRDUP( "&Ocirc;" ); /*   &#212;  capital O, circumflex accent   */
    circs['U'] = STRDUP( "&Ucirc;" ); /*   &#219;  capital U, circumflex accent   */
    circs['a'] = STRDUP( "&acirc;" ); /*   &#226;  small a, circumflex accent   */
    circs['e'] = STRDUP( "&ecirc;" ); /*   &#234;  small e, circumflex accent   */
    circs['i'] = STRDUP( "&icirc;" ); /*   &#238;  small i, circumflex accent   */
    circs['o'] = STRDUP( "&ocirc;" ); /*   &#244;  small o, circumflex accent   */
    circs['u'] = STRDUP( "&ucirc;" ); /*   &#251;  small u, circumflex accent   */
}

/*
  &Aring;  &#197;  capital A, ring  
  &AElig;  &#198;  capital AE diphthong (ligature)  
  &Ccedil;  &#199;  capital C, cedilla  
  &ccedil;  &#231;  small c, cedilla  
  &Oslash;  &#216;  capital O, slash  
  &THORN;  &#222;  capital THORN, Icelandic  
  &aring;  &#229;  small a, ring  
  &aelig;  &#230;  small ae diphthong (ligature)  
  &eth;  &#240;  small eth, Icelandic  
  &oslash;  &#248;  small o, slash  
*/
