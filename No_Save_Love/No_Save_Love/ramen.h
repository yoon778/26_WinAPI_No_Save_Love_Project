#pragma once
class ramen
{
public:
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

