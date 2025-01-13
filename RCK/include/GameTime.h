#pragma once

#include <list>
#include "OutputLog.h"
/**
 * Time is the largest change we're making to the ACKS rules.
 * Since classic Roguelikes work on a system of moves more driven by impulse-movements rather than strict turn taking, concepts like Initiative do not make sense.
 * This requires quite a lot of changes. Most notably, a lot of the system is mediated by means of Initiative (and order of declaration - most notably the spellcasting and defensive movement rules)
 *
 * In order to resolve this, I've decided to use a two-resource approach: Time and Fatigue.
 * Time is measured in Ticks, which represent movement impulses. Since the game defines a map square (containing a single man-sized entity) as being 5ft, that means a single tick is the time taken to move 5ft for the fastest moving speed creature in the game.
 * Each action taken consumes both Ticks and Fatigue. All other entities take their actions when Ticks are advanced. Fatigue can be primarily restored by resting (although we may add some resources elsewhere).
 * Initiative will serve to slightly reduce the amount of time taken by non-movement actions, and determine who goes first where they occur at once, as well as perhaps allowing for reactions if an enemy performs an action while you're undertaking one.
 *
 * This will extensively affect many Proficiencies, which will need to be rewritten for the new system. It will also affect the timing of spellcasting and the operation of defensive movement.
*/

#define MAX_TURN_SPEED 360L
#define MAX_ROUND_SPEED (MAX_TURN_SPEED / 3)
#define BASE_SCALE 5

enum TIME_PERIOD
{
	TIME_ROUND=0,
	TIME_MINUTE,
	TIME_TURN,
	TIME_HOUR,
	TIME_DAY,
	TIME_WEEK,
	TIME_MONTH,
	TIME_YEAR
};

static double long time_periods[] = {
	10.0L,									// 1 round = 10 seconds
	60.0L,									// 1 minute
	60.0L * 10.0L,							// 1 turn = 10 minutes
	60.0L * 60.0L,							// 1 hour
	60.0L * 60.0L * 24.0L,					// 1 day
	60.0L * 60.0L * 24.0L * 7.0L,			// 1 week
	60.0L * 60.0L * 24.0L * 30.0L,			// 1 month
	60.0L * 60.0L * 24.0L * 30.0L * 12.0L	// 1 year
};

struct GameDateTime
{
	int years;
	int months;
	int days;
	int hours;
	int minutes;
	int seconds;
};


class TimeManager
{
	// The time manager maintains the lists of activity counts
	// The primary table is a sorted queue of entity IDs, with the time until they next tick, sorted by time (ascending)
	// When we advance time, we decrement the queue by the length of time of the lowest entity.
	// We store a "round" function for each entity (the player one returns the control to the user). This completes the current action and then decides on the entity's next move.

	std::list<long double> times;
	std::list<int> entities;
	std::list<int> managers;

	void EmplaceEntity(int entityID, int manager, long double time);

	// In addition to the turn handling time management, we also need to handle larger-scale timing events.
	// This is done primarily by the Managers. We count the standing time and inform all the Managers when important time periods pass: Rounds, Turns, Hours, Days, Weeks and Months.
	long double masterTime;
	
	const bool debugMode = true;
	
public:
	TimeManager();;

	bool AdvanceTime(); // advance to the next time element and run its round function
	bool AdvanceTimeBy(long double time);

	void RegisterNewEntity(int entityID, int manager);
	void DeregisterEntities();
	
	void SetEntityTime(int entityID, int manager, long double time);

	void DebugLog(std::string message);

	void DumpTimeToLog(OutputLog* log);
	void DumpTimeToFile(std::string filename);

	long double GetRunningTime();

	GameDateTime GetCalendarTime();

	static long double GetTimePeriodInSeconds(TIME_PERIOD period)
	{
		return time_periods[(int)period];
	}
};
