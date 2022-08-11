#include "GameTime.h"

#include <ctime>
#include <vector>
#include "Game.h"

TimeManager::TimeManager()
{
	masterTime = 0.0L;
}

bool TimeManager::AdvanceTime()
{
	// the first element in the chain is the lowest remaining time

	return AdvanceTimeBy(-1.0L);
}

bool TimeManager::AdvanceTimeBy(long double time)
{
	if (entities.size() <= 0)
		return false;

	// the entities we call are able to interrupt us. If we hit an interruption, we have to quit back out.
	// usually this happens when something important happens to a party member or we need a decision from the player
	// Because we can always be interrupted, we have to deduct time one by one.

	// we cannot modify a collection that is currently being iterated. To correct for this, we need to handle this as a DOD type table transformation.
	// To achieve this we copy the list, and further calls to SetEntityTime (from TurnHandlers) effectively generate a new table.

	std::list<long double > old_times;
	std::list<int> old_entities, old_managers;
	std::copy(times.begin(), times.end(), std::back_inserter(old_times));
	std::copy(entities.begin(), entities.end(), std::back_inserter(old_entities));
	std::copy(managers.begin(), managers.end(), std::back_inserter(old_managers));
	
	std::list<long double >::iterator time_iter = old_times.begin();
	std::list<int>::iterator ent_iter = old_entities.begin();
	std::list<int>::iterator manager_iter = old_managers.begin();

	if (time == -1.0)
	{
		// time of -1 indicates "skip to next event"
		time = *time_iter; // first event
	}

	long double time_elapsed = 0.0L;
	bool result = false;
	for (; time_iter != old_times.end(); time_iter++, ent_iter++, manager_iter++)
	{
		time_elapsed = *time_iter;

		if (time_elapsed > time) break;
		
		switch(*manager_iter)
		{
		case MANAGER_CHARACTER:
			{
				result = gGame->mCharacterManager->TurnHandler(*ent_iter, time_elapsed);
			}
			break;
		case MANAGER_MOB:
			{
				result = gGame->mMobManager->TurnHandler(*ent_iter, time_elapsed);
			}
			break;
		case MANAGER_MAP:
			{
				result = gGame->mMapManager->TurnHandler(*ent_iter, time_elapsed);
			}
			break;
		case MANAGER_ITEM:
			{
				// gGame->mItemManager->TurnHandler(*ent_iter, *time_iter);
			}
			break;
		case MANAGER_BASE:
			{
				result = gGame->mBaseManager->TurnHandler(*ent_iter, time_elapsed);
			}
		}
		
		if (result) break;
	}

	for (long double&t : times)
	{
		t -= time_elapsed;
	}

	// now update the main time counter and call the managers if needed
	long double old_time = masterTime;
	masterTime += time_elapsed;

	int roundsPassed = (int)(masterTime / time_periods[TIME_ROUND]) - (int)(old_time / time_periods[TIME_ROUND]);;

	int turnsPassed = (int)(roundsPassed / 60);
	int hoursPassed = (int)(roundsPassed / 360);
	int daysPassed = (int)(roundsPassed / (360 * 24));
	int weeksPassed = (int)(roundsPassed / (360 * 24 * 7));
	int monthsPassed = (int)(roundsPassed / (360 * 24 * 30));

	// if any time intervals have passed then call the managers to inform them
	// TODO: This doesn't really work as we can have multiple entities with sub-round times, meaning all of them will be lost until we have
	// a gap of more than 10s. Need to pass TimeHandler raw times.
	if(roundsPassed > 0)
	{
		gGame->mCharacterManager->TimeHandler(roundsPassed, turnsPassed, hoursPassed, daysPassed, weeksPassed, monthsPassed);
		gGame->mMapManager->TimeHandler(roundsPassed, turnsPassed, hoursPassed, daysPassed, weeksPassed, monthsPassed);
		gGame->mMobManager->TimeHandler(roundsPassed, turnsPassed, hoursPassed, daysPassed, weeksPassed, monthsPassed);
		gGame->mPartyManager->TimeHandler(roundsPassed, turnsPassed, hoursPassed, daysPassed, weeksPassed, monthsPassed);
		gGame->mBaseManager->TimeHandler(roundsPassed, turnsPassed, hoursPassed, daysPassed, weeksPassed, monthsPassed);
	}

	// advance by intervals. 

	if(debugMode)
	{
		DumpTimeToFile("time_dump.txt");
	}
	
	return result;
}

GameDateTime TimeManager::GetCalendarTime()
{
	GameDateTime out;

	long double ctime = masterTime;

	long double year = GetTimePeriodInSeconds(TIME_YEAR);

	out.years = (int)(ctime / year);
	ctime = fmod(ctime, year);

	long double month = GetTimePeriodInSeconds(TIME_MONTH);

	out.months = (int)(ctime / month);
	ctime = fmod(ctime, month);

	long double day = GetTimePeriodInSeconds(TIME_DAY);

	out.days = (int)(ctime / day);
	ctime = fmod(ctime, day);

	long double hour = GetTimePeriodInSeconds(TIME_HOUR);

	out.hours = (int)(ctime / hour);
	ctime = fmod(ctime, hour);

	long double minute = GetTimePeriodInSeconds(TIME_MINUTE);

	out.minutes = (int)(ctime / minute);
	ctime = fmod(ctime, minute);

	out.seconds = (int)(fabs(ctime));

	return out;
}

void TimeManager::RegisterNewEntity(int entityID, int manager)
{
	long double newTime = 0.01L; // close to immediately, go to handler to make decisions
	EmplaceEntity(entityID, manager, newTime);
}

void TimeManager::DeregisterEntities()
{
	// generally used when we change maps. This does not affect long term timing (since that calls the managers directly)
	entities.clear();
	managers.clear();
	times.clear();
}

void TimeManager::SetEntityTime(int entityID, int manager, long double time)
{
	std::list<int>::iterator ent_iter = entities.begin();
	std::list<int>::iterator manager_iter = managers.begin();
	std::list<long double>::iterator time_iter = times.begin();
	
	// wind forward to the entity we're looking for
	for (; ent_iter != entities.end() && !(*ent_iter == entityID && *manager_iter == manager); ent_iter++, manager_iter++, time_iter++)
	{}

	if (ent_iter != entities.end())
	{
		entities.erase(ent_iter);
		managers.erase(manager_iter);
		times.erase(time_iter);
	}
	EmplaceEntity(entityID, manager, time);
}

void TimeManager::EmplaceEntity(int entityID, int manager, long double time)
{
	std::list<int>::iterator ent_iter = entities.begin();
	std::list<int>::iterator manager_iter = managers.begin();
	std::list<long double>::iterator time_iter = times.begin();

	for(; time_iter != times.end()  && *time_iter < time ;ent_iter++,manager_iter++,time_iter++)
	{}

	entities.emplace(ent_iter, entityID);
	managers.emplace(manager_iter, manager);
	times.emplace(time_iter, time);

	if (debugMode)
	{
		DumpTimeToFile("time_dump.txt");
	}
}

void TimeManager::DebugLog(std::string message)
{
	gLog->Log("TimeManager", message);
}

void TimeManager::DumpTimeToLog(OutputLog* log)
{
	log->Log("TimeManager", "=============");
	log->Log("TimeManager", "DUMPING TIMES");
	log->Log("TimeManager", "=============");

	std::list<int>::iterator ent_iter = entities.begin();
	std::list<int>::iterator manager_iter = managers.begin();
	std::list<long double>::iterator time_iter = times.begin();

	for (; time_iter != times.end(); ent_iter++, manager_iter++, time_iter++)
	{
		switch (*manager_iter)
		{
		case MANAGER_CHARACTER:
		{
			std::string name = gGame->mCharacterManager->getCharacterName(*ent_iter);
			log->Log("TimeManager", "Character(" + name + "), " + std::to_string(*time_iter));
		}
		break;
		case MANAGER_MOB:
		{
			std::string name = gGame->mMobManager->GetMonster(*ent_iter).GetName();
			log->Log("TimeManager", "Monster(" + name + "), " + std::to_string(*time_iter));
		}
		break;
		case MANAGER_MAP:
		{
			//gGame->mMapManager->TurnHandler(*ent_iter, time_elapsed);
		}
		break;
		case MANAGER_ITEM:
		{
			// gGame->mItemManager->TurnHandler(*ent_iter, *time_iter);
		}
		break;
		}
	}
}

void TimeManager::DumpTimeToFile(std::string filename)
{
	OutputLog* log = new OutputLog(filename);

	DumpTimeToLog(log);

	delete log;
}

long double TimeManager::GetRunningTime()
{
	return masterTime;
}
