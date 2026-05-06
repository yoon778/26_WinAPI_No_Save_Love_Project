#pragma once
class ramen
{
public:
	ramen();
	bool water;
	bool noodle;
	bool soup;
	bool egg;
	bool cheese;
	bool dumpling;


	void clear();
	void makebasicramen();
	void makerandramen();
	bool ramenSame(const ramen& other);
};

