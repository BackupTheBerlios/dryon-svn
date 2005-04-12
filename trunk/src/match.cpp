/*
********************************************************************************
*                                 Dryon project
********************************************************************************
*
*  This is free software under the GPL license.
*  Since this software uses Small from Compuphase, it also use its license.
*  Copy of the two licenses are in the main dir (gpl.txt and amx_license.txt
*  Coded by Ammous Julien known as Anthalir
*
********************************************************************************
*/
/*
part of this file come from PuTTY:

PuTTY is copyright 1997-2004 Simon Tatham.

Portions copyright Robert de Bath, Joris van Rantwijk, Delian
Delchev, Andreas Schultz, Jeroen Massar, Wez Furlong, Nicolas Barry,
Justin Bradford, Ben Harris, and CORE SDI S.A.

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

//#include <stdio.h>

#include "config.h"
#if defined(__FreeBSD__)
#include <sys/types.h>
#endif

#include <stdio.h>
#include <string>
#include "match.h"



enum {
    WC_TRAILINGBACKSLASH = 1,
    WC_UNCLOSEDCLASS,
    WC_INVALIDRANGE
};

/*
 * Definition of wildcard syntax:
 *
 *  - ' ' matches any number of spaces or tabulations
 *  - '*' matches any sequence of characters, including zero.
 *  - '?' matches exactly one character which can be anything.
 *  - [abc] matches exactly one character which is a, b or c.
 *  - [a-f] matches anything from a through f.
 *  - [^a-f] matches anything _except_ a through f.
 *  - [-_] matches - or _; [^-_] matches anything else. (The - is
 *    non-special if it occurs immediately after the opening
 *    bracket or ^.)
 *  - [a^] matches an a or a ^. (The ^ is non-special if it does
 *    _not_ occur immediately after the opening bracket.)
 *  - \*, \?, \[, \], \\ match the single characters *, ?, [, ], \.
 *  - All other characters are non-special and match themselves.
 */
static int wc_match_fragment(const char **fragment, const char **target)
{
    const char *f, *t;

    f = *fragment;
    t = *target;
    /*
     * The fragment terminates at either the end of the string, or
     * the first (unescaped) *.
     */
    while( *f && *f != '*' && *t )
    {
		/*
		 * Extract one character from t, and one character's worth
		 * of pattern from f, and step along both. Return 0 if they
		 * fail to match.
		 */
		if( *f == '\\' )
		{
		    /*
		     * Backslash, which means f[1] is to be treated as a
		     * literal character no matter what it is. It may not
		     * be the end of the string.
		     */
		    if( !f[1] )
				return -WC_TRAILINGBACKSLASH;   /* error */

		    if( f[1] != *t )
				return 0;	       /* failed to match */

		    f+= 2;
		}
		// Question mark matches anything.
		else if( *f == '?' )
		{
		    f++;
		}
		// space match any number of spaces/tabs
		else if( *f == ' ' )
		{
			if( (*t == ' ') || (*t == '\t') )
			{
				if( !*(t+1) )
					f++;
			}
			else
			{
				while( *f && (*f == ' ') )
					f++;

				continue;
			}
		}
/*
		// Open bracket introduces a character class.
		else if( *f == '[' )
		{
		    int invert= 0;
		    int matched= 0;

		    f++;
		    if( *f == '^' )
		    {
				invert= 1;
				f++;
		    }

		    while( *f != ']' )
		    {
				if( *f == '\\' )
				    f++;	       // backslashes still work

				if( !*f )
				    return -WC_UNCLOSEDCLASS;   // error again

				if( f[1] == '-' )
				{
				    int lower, upper, ourchr;
				    lower= (unsigned char) *f++;
				    f++;	       // eat the minus
				    if( *f == ']' )
						return -WC_INVALIDRANGE;   // different error!

				    if( *f == '\\' )
						f++;	       // backslashes _still_ work

				    if( !*f )
						return -WC_UNCLOSEDCLASS;   // error again

				    upper= (unsigned char) *f++;
				    ourchr= (unsigned char) *t;
				    if( lower > upper )
				    {
						int t= lower; lower = upper; upper = t;
				    }

				    if( ourchr >= lower && ourchr <= upper )
						matched = 1;
				}
				else
				{
				    matched|= (*t == *f++);
				}
		    }
		    if( invert == matched )
				return 0;	       // failed to match character class

		    f++;		       // eat the ]
		}
*/
		else
		{
			// Non-special character matches itself.
		    if( *f != *t )
				return 0;

		    f++;
		}
		/*
		 * Now we've done that, increment t past the character we
		 * matched.
		 */
		t++;
    }

    if( !*f || (*f == '*') )
    {
		/*
		 * We have reached the end of f without finding a mismatch;
		 * so we're done. Update the caller pointers and return 1.
		 */
		*fragment= f;
		*target= t;
		return 1;
    }
    /*
     * Otherwise, we must have reached the end of t before we
     * reached the end of f; so we've failed. Return 0.
     */
    return 0;
}

/*
 * This is the real wildcard matching routine. It returns 1 for a
 * successful match, 0 for an unsuccessful match, and <0 for a
 * syntax error in the wildcard.
 */
int wc_match(const string &_wildcard, const string &_target)
{
    int ret;

    const char *wildcard= _wildcard.c_str();
    const char *target= _target.c_str();

    /*
     * Every time we see a '*' _followed_ by a fragment, we just
     * search along the string for a location at which the fragment
     * matches. The only special case is when we see a fragment
     * right at the start, in which case we just call the matching
     * routine once and give up if it fails.
     */
    if( *wildcard != '*' )
    {
		ret= wc_match_fragment(&wildcard, &target);
		if (ret<= 0)
			return ret;		       /* pass back failure or error alike */
    }

    while( *wildcard )
    {
		assert(*wildcard == '*');
		while( *wildcard == '*' )
		    wildcard++;

		/*
		 * It's possible we've just hit the end of the wildcard
		 * after seeing a *, in which case there's no need to
		 * bother searching any more because we've won.
		 */
		if( !*wildcard )
		    return 1;

		/*
		 * Now `wildcard' points at the next fragment. So we
		 * attempt to match it against `target', and if that fails
		 * we increment `target' and try again, and so on. When we
		 * find we're about to try matching against the empty
		 * string, we give up and return 0.
		 */
		ret= 0;
		while( *target )
		{
		    const char *save_w= wildcard, *save_t= target;

		    ret= wc_match_fragment(&wildcard, &target);

		    if( ret < 0 )
				return ret;	       /* syntax error */

		    if( (ret > 0) && !*wildcard && *target)
		    {
				/*
				 * Final special case - literally.
				 *
				 * This situation arises when we are matching a
				 * _terminal_ fragment of the wildcard (that is,
				 * there is nothing after it, e.g. "*a"), and it
				 * has matched _too early_. For example, matching
				 * "*a" against "parka" will match the "a" fragment
				 * against the _first_ a, and then (if it weren't
				 * for this special case) matching would fail
				 * because we're at the end of the wildcard but not
				 * at the end of the target string.
				 *
				 * In this case what we must do is measure the
				 * length of the fragment in the target (which is
				 * why we saved `target'), jump straight to that
				 * distance from the end of the string using
				 * strlen, and match the same fragment again there
				 * (which is why we saved `wildcard'). Then we
				 * return whatever that operation returns.
				 */
				target= save_t + strlen(save_t) - (target - save_t);
				wildcard= save_w;
				return wc_match_fragment(&wildcard, &target);
		    }

		    if( ret > 0 )
				break;
		    target++;
		}
		if( ret > 0 )
		    continue;
		return 0;
    }

    /*
     * If we reach here, it must be because we successfully matched
     * a fragment and then found ourselves right at the end of the
     * wildcard. Hence, we return 1 if and only if we are also
     * right at the end of the target.
     */
    return (*target ? 0 : 1);
}

// "donne une * à *"
// "donne   une pomme à Bob"

// ' ' = one or more space (or tabs)
// '*' = anything
// any other character match only itself
bool match_expr(const string &mask, const string &str, string *results, uint maxresults)
{
	uint cur= 0, cur_result= 0;
	bool lastm= false, lasts= false;
	char m, c;
	string saved_str;

	saved_str.reserve(100);

	for(uint i= 0;; i++)
	{
		lastm= ( cur == mask.length()-1 );
		lasts= ( i == str.length()-1 );

		m= mask[cur]; c= str[i];

		if( (m!='*') && ((cur >= mask.length()) || (i >= str.length())) )
			return false;


		if( m=='*' )
		{
			saved_str+= c;

			//if( lastm ) return true;	// * is the last character in mask
			if( !lastm && lasts ) return false;	// * is not the last character in mask but end of str is reached

			if( (mask[cur+1]==str[i+1]) || ((mask[cur+1]==' ') && (str[i+1]=='\t')) )
			{
				if( cur_result < maxresults )
				{
					//printf("string[*]: '%s'\n", saved_str.c_str());
					results[cur_result]= saved_str;
					cur_result++;
					saved_str.erase();
				}

				cur++;

				if( str[i+1]=='\0' )
					return true;
			}

			continue;
		}

		if( lastm && lasts && (c==m) )
			return true;

		if( m==' ' )
		{
			if( (c==' ') || (c=='\t') )
			{
				// no other space/tab follow the current one in str
				if( !lasts && (str[i+1]!=' ') && (str[i+1]!='\t') )
					cur++;

				continue;
			}
		}
		else
		{
			if( m==c )
				cur++;
			else
				return false;
		}
	}

	return false;
}


#ifdef TEST

#include <stdio.h>
//#define TESTMATCH(M,E) if( match_expr(M,E) ) printf("- true  : mask('"##M##"') str('"##E##"')\n"); else printf("- false : mask('"##M##"') str('"##E##"')\n");
#define TESTMATCH(M,E) if( wc_match(M,E) ) printf("- true  : mask('"##M##"') str('"##E##"')\n"); else printf("- false : mask('"##M##"') str('"##E##"')\n");

void main()
{
	printf("\nshould be FALSE:\n");
    TESTMATCH("*a*", "perk");
    TESTMATCH("?b*r?", "abracadabr");
    TESTMATCH("?b*r?", "abracadabzr");
    TESTMATCH("a*", "badger");
    TESTMATCH("*a", "park");
    TESTMATCH("a", "ba");
	TESTMATCH("a", "argh");
	TESTMATCH("cb*", "cd");
	TESTMATCH("donne un cookie à *", "donne");
	TESTMATCH("un * deux", "un jj");
	TESTMATCH("dans * * bocal", "dans un ou bocail");;

	printf("\nshould be TRUE:\n");
    TESTMATCH("?b*r?", "abracadabra");
    TESTMATCH("*a", "pArka");
    TESTMATCH("*a", "parka");
    TESTMATCH("*a*", "park");
    TESTMATCH("a", "a");
    TESTMATCH("a*", "aardvark");
	TESTMATCH("a", "a");
	TESTMATCH("cb*", "cb");
	TESTMATCH("a b", "a			b");
	TESTMATCH("*", "un chat");
	TESTMATCH("un *", "un chat");
	TESTMATCH("* chat", "deux chat");
	TESTMATCH("* chat", "trois chat");
	TESTMATCH("un chat dans *", "un chat    dans la   rue");
	TESTMATCH("un ", "un ");
	TESTMATCH("dans * bocal", "dans un bocal");
	TESTMATCH("FtS_mC@*wanadoo.fr", "FtS_mC@ADijon-106-1-7-222.w81-48.abo.wanadoo.fr");

/*
	string strs[5];
	bool r= match_expr("* * *", "testbot		*!*@gresillons-2-82-67-175-16.fbx.proxad.net 	o", strs, 3);
	printf("r= %d\n", (r)?1:0);

	for(int i= 0; i< 3; i++)
		printf("%d: %s\n", i, strs[i].c_str());
*/
	return 0;
}

#endif
