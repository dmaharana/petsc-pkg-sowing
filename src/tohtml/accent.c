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
    umlauts['A']  = strdup( "&#196;" ); /* &Auml;  capital A, dieresis or umlaut mark */
    umlauts['E']  = strdup( "&#203;" ); /* &Euml;  &#203;  capital E, dieresis or umlaut mark  */
    umlauts['I']  = strdup( "&#207;" ); /* &Iuml;  &#207;  capital I, dieresis or umlaut mark  */
    umlauts['O']  = strdup( "&#214;" ); /* &Ouml;  &#214;  capital O, dieresis or umlaut mark  */
    umlauts['U']  = strdup( "&#220;" ); /* &Uuml;  &#220;  capital U, dieresis or umlaut mark  */
    umlauts['a']  = strdup( "&#228;" ); /* &auml;  &#228;  small a, dieresis or umlaut mark    */
    umlauts['e']  = strdup( "&#235;" ); /* &euml;  &#235;  small e, dieresis or umlaut mark    */
    umlauts['i']  = strdup( "&#239;" ); /* &iuml;  &#239;  small i, dieresis or umlaut mark    */
    umlauts['o']  = strdup( "&#246;" ); /* &ouml;  &#246;  small o, dieresis or umlaut mark    */
    umlauts['u']  = strdup( "&#252;" ); /* &uuml;  &#252;  small u, dieresis or umlaut mark    */
    umlauts['y']  = strdup( "&#255;" ); /* &yuml;  &#255;  small y, dieresis or umlaut mark    */
}

static void InitGraves( void )
{
    graves['A'] = strdup( "&#192;" ); /* &Agrave;  &#192;  capital A, grave accent  */
    graves['E'] = strdup( "&#200;" ); /* &Egrave;  &#200;  capital E, grave accent  */
    graves['I'] = strdup( "&#204;" ); /* &Igrave;  &#204;  capital I, grave accent  */
    graves['O'] = strdup( "&#210;" ); /* &Ograve;  &#210;  capital O, grave accent  */
    graves['U'] = strdup( "&#217;" ); /* &Ugrave;  &#217;  capital U, grave accent  */
    graves['a'] = strdup( "&#224;" ); /* &agrave;  &#224;  small a, grave accent    */
    graves['e'] = strdup( "&#232;" ); /* &egrave;  &#232;  small e, grave accent    */
    graves['i'] = strdup( "&#236;" ); /* &igrave;  &#236;  small i, grave accent    */
    graves['o'] = strdup( "&#242;" ); /* &ograve;  &#242;  small o, grave accent    */
    graves['u'] = strdup( "&#249;" ); /* &ugrave;  &#249;  small u, grave accent    */
}

static void InitAcutes( void )
{
    acutes['A'] = strdup( "&#193;" ); /* &Aacute;  &#193;  capital A, acute accent   */
    acutes['E'] = strdup( "&#201;" ); /* &Eacute;  &#201;  capital E, acute accent   */
    acutes['I'] = strdup( "&#205;" ); /* &Iacute;  &#205;  capital I, acute accent   */
    acutes['O'] = strdup( "&#211;" ); /* &Oacute;  &#211;  capital O, acute accent   */
    acutes['U'] = strdup( "&#218;" ); /* &Uacute;  &#218;  capital U, acute accent   */
    acutes['Y'] = strdup( "&#221;" ); /* &Yacute;  &#221;  capital Y, acute accent   */
    acutes['a'] = strdup( "&#225;" ); /* &aacute;  &#225;  small a, acute accent     */
    acutes['e'] = strdup( "&#233;" ); /* &eacute;  &#233;  small e, acute accent     */
    acutes['i'] = strdup( "&#237;" ); /* &iacute;  &#237;  small i, acute accent     */
    acutes['o'] = strdup( "&#243;" ); /* &oacute;  &#243;  small o, acute accent     */
    acutes['u'] = strdup( "&#250;" ); /* &uacute;  &#250;  small u, acute accent     */
    acutes['y'] = strdup( "&#253;" ); /* &yacute;  &#253;  small y, acute accent     */
}

static void InitTildes( void )
{
    tildes['A'] = strdup( "&#195;" ); /* &Atilde;  &#195;  capital A, tilde   */
    tildes['N'] = strdup( "&#209;" ); /* &Ntilde;  &#209;  capital N, tilde   */
    tildes['O'] = strdup( "&#213;" ); /* &Otilde;  &#213;  capital O, tilde   */
    tildes['a'] = strdup( "&#227;" ); /* &atilde;  &#227;  small a, tilde     */
    tildes['n'] = strdup( "&#241;" ); /* &ntilde;  &#241;  small n, tilde     */
    tildes['o'] = strdup( "&#245;" ); /* &otilde;  &#245;  small o, tilde     */
}

static void InitCircs( void )
{
    circs['A'] = strdup( "&Acirc;" ); /*   &#194;  capital A, circumflex accent   */
    circs['E'] = strdup( "&Ecirc;" ); /*   &#202;  capital E, circumflex accent   */
    circs['I'] = strdup( "&Icirc;" ); /*   &#206;  capital I, circumflex accent   */
    circs['O'] = strdup( "&Ocirc;" ); /*   &#212;  capital O, circumflex accent   */
    circs['U'] = strdup( "&Ucirc;" ); /*   &#219;  capital U, circumflex accent   */
    circs['a'] = strdup( "&acirc;" ); /*   &#226;  small a, circumflex accent   */
    circs['e'] = strdup( "&ecirc;" ); /*   &#234;  small e, circumflex accent   */
    circs['i'] = strdup( "&icirc;" ); /*   &#238;  small i, circumflex accent   */
    circs['o'] = strdup( "&ocirc;" ); /*   &#244;  small o, circumflex accent   */
    circs['u'] = strdup( "&ucirc;" ); /*   &#251;  small u, circumflex accent   */
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
