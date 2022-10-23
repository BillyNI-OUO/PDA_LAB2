#ifndef CONTOUR_H
#define CONTOUR_H
#include <iostream>
#include <list>
#include <iterator>


typedef struct{
	int x;
	int y;
	int width;
	int height;
}contour;

class Contour{
	public:
		std::list<contour> l;
		void PrintContent(){
			list<Point>::iterator it;
    		for (it = this->l.begin(); it != this->l.end(); ++it){
        		std::cout << "(x, y): " << it->x << ", " << it->y << std::endl;
    		}
		};
		void Insert(Block block);
};

#endif