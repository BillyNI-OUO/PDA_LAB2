#ifndef BLOCK_H
#define BLOCK_H 

#include <utility>
#include <iostream>
#include <string>

class Block{
	public:
		int width, height;
		std::string name;
		int index;
		Block (int width, int height, std::string name, int index);
		Block(){};
		void Swap();
		void PrintContent(){
			std::cout << "width: " << width << std::endl;
			std::cout << "height: " << height << std::endl;
			std::cout << "name: " << name << std::endl;
			std::cout << "index: " << index << std::endl;
		};
};

#endif