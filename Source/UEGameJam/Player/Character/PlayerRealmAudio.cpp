#include "Player/Character/GsPlayer.h"

#include "Audio/BgmSubsystem.h"
#include "RealmTagComponent.h"

void AGsPlayer::SyncBGMWithCurrentRealm()
{
	UBgmSubsystem* BgmSubsystem = UBgmSubsystem::Get(this);
	if (!BgmSubsystem)
	{
		return;
	}

	const bool bInsideRealm = IsInsideActiveRealmReveal();
	if (bHasSyncedBGMRealm && bInsideRealm == bLastInsideRealmForBGM)
	{
		return;
	}

	bHasSyncedBGMRealm = true;
	bLastInsideRealmForBGM = bInsideRealm;
	BgmSubsystem->SwitchCombatBGM(bInsideRealm ? ERealmType::Realm : ERealmType::Surface);
}
