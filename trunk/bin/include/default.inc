/*

Codes mirc:

couleur: 	\003x
	00 white	08  yellow
	01 black	09  lightgreen
	02 blue		10 cyan
	03 green	11 lightcyan
	04 red		12 lightblue
	05 brown	13 pink
	06 purple	14 grey
	07 orange	15 lightgrey
	99 or nothing: previous color

gras:		\002
souligné:	\037

*/

#include <amxcore>
#include <amxbot>

public max_nicklen;
public max_topiclen;

stock strcpy(dest[], const src[], bool:packed= false){ return strncpy(dest, src, -1, packed); }
stock bool:strmatch(const s1[], const s2[]){ return strnmatch(s1,s2,-1); }
stock bool:strmatchi(const s1[], const s2[]){ return strnmatchi(s1,s2,-1); }
/*
stock copyc(in[],start,out[],chr)
{
   new j=0;
   for (new i=start;i<=strlen(in);i++)
   {
      if (in[i] != chr)
      {
         out[j] = in[i];
         j++;
      }
      else
      {
         i=strlen(in);
      }
   }
   return j;
}
*/
stock exp(x,y)
{
   new r= 1;
   for(new i= 0; i< y; i++)
   {
      r*= x;
   }

   return r;
}

stock bool:ispacked(const string[])
{
	return !(charmin <= string[0] <= charmax);
}
/*
stock findchar(const str[], from, c)
{
	if( ispacked(str) )
	{
		for(new i= from; i< strlen(str); i++ )
		{
			if( str{i}==c )
				return i;
		}
	}
	else
	{
		for(new i= from; i< strlen(str); i++ )
		{
			if( str[i]==c )
				return i;
		}
	}

	return -1;
}
*/
stock strncpy2(dest[], start, const src[], size)
{
	for(new i= 0; i< size; i++)
		dest[start+i]= src[i];

	dest[start+size]= 0;
}

stock strcat(s1[], const s2[])
{
	strncpy2(s1, strlen(s1), s2, strlen(s2));
}

stock bool:in_range(x, min_val, max_val)
{
	return ((x>=min_val) && (x<=max_val));
}


/**/

