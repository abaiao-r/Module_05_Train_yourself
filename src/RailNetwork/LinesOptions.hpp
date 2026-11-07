#ifndef LINESOPTIONS_HPP
#define LINESOPTIONS_HPP

/**
 * Bonus switch for multi-line rails with head-on avoidance (--lines).
 * Disabled by default: the mandatory behaviour is untouched unless a
 * caller opts in explicitly.
 */
struct LinesOptions
{
	bool enabled = false;
};

#endif
