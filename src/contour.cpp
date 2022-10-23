#include "contour.h"
#include "block.h"
#include <iostream>
#include <list>
#include <iterator>


void Contour::Insert(Block block){
	list<contour>::iterator it;
    for (it = this.l.begin(); it != this.l.end(); ++it){
    	if(it->x > block.leftCornerX)
    }
}