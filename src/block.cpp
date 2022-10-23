#include "block.h"
#include <iostream>
#include <string>
#include <algorithm>
Block::Block(int width, int height, std::string name, int index){
	this->width = width;
	this->height = height;
	this->name = name;
	this->index = index;
}

void Block::Swap(){
	std::swap(this->height, this->width);
}