#pragma once

#include <string>

#include "aspect.h"
#include "name.h"
#include "script.h"

class Character;

class Gauge : public Named {
public:
	Gauge(
		Character * character,
		Name name,
		unsigned int initVal,
		unsigned int max,
		const Aspect& aFull,
		const Aspect& aEmpty,
		bool visible = true);
	// Automatically add itself to its character.
	~Gauge();
	unsigned int getVal();
	void setVal(unsigned int val);
	void increase(unsigned int val);
	void decrease(unsigned int val);
	unsigned int getMax();
	void setMax(unsigned int max);

	/* Scripts */
	const Script& getWhenFull();
	void setWhenFull(const Script& script);
	void resetWhenFull();
	const Script& getWhenEmpty();
	void setWhenEmpty(const Script& script);
	void resetWhenEmpty();

	bool isVisible();
	void setVisible();
	void setNotVisible();

private:
	class Character * character;
	unsigned int val;
	unsigned int max;
	Script whenFull;
	Script whenEmpty;
	Aspect aFull;
	Aspect aEmpty;
	bool visible;

	void update();
	void disapear();
	void exeFull();
	void exeEmpty();
};
