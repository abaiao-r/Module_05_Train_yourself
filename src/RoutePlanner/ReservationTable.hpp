#ifndef RESERVATIONTABLE_HPP
#define RESERVATIONTABLE_HPP

#include <algorithm>
#include <map>
#include <string>
#include <vector>

/**
 * Who is planned to be where, and when: a (rail, lane, direction) is
 * reserved for a half-open window [t0, t1) in seconds.
 *
 * Conflict rule (matches the line-admission rule of --lines mode):
 *   - opposite directions on the same lane conflict;
 *   - the same direction on the same lane does NOT conflict (trains
 *     follow each other) but is reported by sameDirectionOverlap() so a
 *     planner can price the queueing;
 *   - different lanes never conflict.
 * Without head-on exclusion (legacy mode) nothing ever conflicts.
 */
class ReservationTable
{
  public:
	explicit ReservationTable(bool headOnExclusion = true)
		: _headOn(headOnExclusion) {}

	void reserve(const std::string &from, const std::string &to, int lane,
				 double t0, double t1)
	{
		_slots[key(from, to)].push_back({lane, direction(from, to), t0, t1});
	}

	/** True if a train may occupy from->to on `lane` during [t0, t1). */
	bool isFree(const std::string &from, const std::string &to, int lane,
				double t0, double t1) const
	{
		return conflictEnd(from, to, lane, t0, t1) < 0.0;
	}

	/** Earliest start >= t at which the lane is free for `duration`. */
	double earliestFree(const std::string &from, const std::string &to,
						int lane, double t, double duration) const
	{
		double start = t;
		for (double end = conflictEnd(from, to, lane, start, start + duration);
			 end >= 0.0;
			 end = conflictEnd(from, to, lane, start, start + duration))
			start = end;
		return start;
	}

	/** Seconds of [t0, t1) already used by trains going the SAME way. */
	double sameDirectionOverlap(const std::string &from, const std::string &to,
								int lane, double t0, double t1) const
	{
		double total = 0.0;
		auto it = _slots.find(key(from, to));
		if (it == _slots.end())
			return 0.0;
		int dir = direction(from, to);
		for (const Slot &s : it->second)
			if (s.lane == lane && s.dir == dir)
				total += std::max(0.0, std::min(t1, s.t1) - std::max(t0, s.t0));
		return total;
	}

	bool empty() const { return _slots.empty(); }

  private:
	struct Slot
	{
		int lane;
		int dir;
		double t0;
		double t1;
	};

	static std::string key(const std::string &a, const std::string &b)
	{
		return a < b ? a + "|" + b : b + "|" + a;
	}
	static int direction(const std::string &from, const std::string &to)
	{
		return from < to ? 0 : 1;
	}

	/* End of the latest conflicting window, or -1 when there is none. */
	double conflictEnd(const std::string &from, const std::string &to,
					   int lane, double t0, double t1) const
	{
		if (!_headOn)
			return -1.0;
		auto it = _slots.find(key(from, to));
		if (it == _slots.end())
			return -1.0;
		int dir = direction(from, to);
		double end = -1.0;
		for (const Slot &s : it->second)
			if (s.lane == lane && s.dir != dir && s.t0 < t1 && t0 < s.t1)
				end = std::max(end, s.t1);
		return end;
	}

	bool _headOn;
	std::map<std::string, std::vector<Slot>> _slots;
};

#endif
