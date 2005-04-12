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

#ifndef __RUBY_EMBED
#define __RUBY_EMBED

extern "C"
{
	void Init_Embed(void);
};

struct swig_class {
  VALUE klass;
  VALUE mImpl;
  void  (*mark)(void *);
  void  (*destroy)(void *);
};

extern swig_class cUser;
extern swig_class cChannel;

#endif // __RUBY_EMBED

/**/


