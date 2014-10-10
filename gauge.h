#ifndef GAUGE_H
#define GAUGE_H

#include <string>

#include "aspect.h"

class Player;

class Gauge {
	private:
		class Player * player;
		std::string name;
		unsigned int val;
		unsigned int max;
		std::string onFull;
		std::string onEmpty;
		Aspect aFull;
		Aspect aEmpty;
		bool visible;

		void update();
		void disapear();
		void exeFull();
		void exeEmpty();
	public:
		Gauge(
			Player * player,
			std::string name,
			unsigned int initVal,
			unsigned int max,
			Aspect aFull,
			Aspect aEmpty,
			bool visible = true);
		~Gauge();
		std::string getName();
		void setName(std::string name);
		unsigned int getVal();
		void setVal(unsigned int val);
		void increase(unsigned int val);
		void decrease(unsigned int val);
		unsigned int getMax();
		void setMax(unsigned int max);
		std::string getOnFull();
		void setOnFull(std::string script);
		void resetOnFull();
		std::string getOnEmpty();
		void setOnEmpty(std::string script);
		void resetOnEmpty();
		bool isVisible();
		void setVisible();
		void setNotVisible();
};

#endif
