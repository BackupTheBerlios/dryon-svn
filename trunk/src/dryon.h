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

/*!
\mainpage Dryon documentation home
\section platform supported
	- FreeBSD (fully tested since i coded it mainly for it)
	- Linux (need more testing)
	- Windows (need more testing)
*/

/*!
\file dryon.h
\author Anthalir
*/

#ifndef _DRYON_H
#define _DRYON_H

#include <stdio.h>
#include "basebot.h"
#include "amx.h"

/*********************/

// sc_comp.cpp
char * AMXAPI amx_StrError(int errnum);
size_t srun_ProgramSize(const char *filename);
int srun_LoadProgram(AMX *amx, const char *filename, void *memblock);
int srun_BuildScript(const char *name);

extern "C"
{
	int amx_CoreInit(AMX*);
	int AMXAPI amx_Callback(AMX *amx, cell index, cell *result, cell *params);
}

/*!
\brief bot's main class
*/
class DryonBot : public BaseBot
{
private:
	void checkUserFlags(user_Info*, const string &dest);
	bool welcome_received;

public:
	void DryonBot::updateFlags(const string &mask);
	~DryonBot();
	bool alreadyConnected(){ return welcome_received; }
	void onConnected();
	void onRegistered();
	void onBotExit();
	void onJoin(const string&, const string&);
	void onPart(const string&, const string&);
	void onQuit(const string&, const string&);
	void onPrivMsg(const string&, const string&, const string&);
	void onKick(const string&, const string&, const string&, const string&);
	void onTopicChange(const string&, const string&, const string&);
	void onNickChange(const string&, const string&);
	void onCTCPAction(const string&, const string&, const string&);

	void onPing(const string&);

	void onBotJoined(	const string &channel, const vector<string> &userlist);
	void onServerOp(const string &channel, const string &target);
	void onServerRemoveOp(	const string &channel, const string &target);
	void onServerVoice(const string &channel, const string &target);
	void onServerRemoveVoice(	const string &channel, const string &target);
	void onBotGainOp(	const string &channel, const string &sender);
	void onBotLostOp(	const string &channel, const string &sender);
	void onBotGainVoice(const string &channel, const string &sender);
	void onBotLostVoice(const string &channel, const string &sender);
	void onBotBanned(	const string &channel, const string &sender, const string &banmask);
	void onBotUnBanned( const string &channel, const string &sender, const string &banmask);

	void onGainOp(	 const string &channel, const string &sender, const string &target);
	void onLostOp(	 const string &channel, const string &sender, const string &target);
	void onGainVoice(const string &channel, const string &sender, const string &target);
	void onLostVoice(const string &channel, const string &sender, const string &target);
	void onBanned(	 const string &channel, const string &sender, const string &banmask);
	void onUnBan(	 const string &channel, const string &sender, const string &banmask);

	void onChanKeySet(const string &channel, const string &sender, const string &key);
	void onChanKeyRemoved(const string &channel, const string &sender);
	void onChanModeChanged(const string &sender, const string &channel, const string &mode);
};

#endif // _DRYON_H

/**/

