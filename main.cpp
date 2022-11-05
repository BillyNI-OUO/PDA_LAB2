#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>
#include <random>
#include <climits>
#include <float.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include "./src/block.cpp"

using namespace std;
double T = 4000000000;
double r = 0.85;
double penalty = 1.3;
int failCnt = 0;
int root, preRoot, bestRoot;
int outLineWidth, outLineHeight;
int blockSize, terminalSize;
vector<Block> blocks;
unordered_map<string, int> nameMap;
vector<int> leftChild, rightChild, parent;
vector<int> preivousTree;
vector<int> preLeftChild, preRightChild, preParent;
vector<vector<int>>bestBlocksPlacement;
vector<Block> preBlocks;
vector<Block> bestBlocks;
double preCost = DBL_MAX;
double bestCost = DBL_MAX;
int bestArea = -1;
int bestWidth = -1;
int bestHeight = -1;
int bestHPWL = -1;
double alpha = 0.5;
int failture  = 0;
bool timeFlag = true;
clock_t startTime, endTime;

void parseBlocks(char* fileName, vector<pair<int, int>>& terminals);
void parseNets(char* fileName, vector<vector<int>>& nets);
void BTreeInit(vector<int>& currentTree, vector<vector<int>>nets, vector<pair<int, int>> terminals);
void SA(vector<vector<int>>nets, vector<pair<int, int>> terminals, vector<int>& currentTree);
void updateContour(int current, int preivous, vector<pair<int, int>>& lowerLeft, vector<pair<int, int>>& upperRight, vector<int> &forward, vector<Block> blocks, vector<int> &backward);
void dfsUpdate(int curBlock, int preBlock, vector<pair<int, int>>& lowerLeft, vector<pair<int, int>>& upperRight, vector<int> &forward, vector<Block>& blocks, vector<int> &backward);
long countHPWL(vector<vector<int>>nets, vector<pair<int, int>> terminals, vector<pair<int, int>> lowerLeft, vector<pair<int, int>> upperRight);
void reflesh(vector<vector<int>>nets, vector<pair<int, int>> terminals, vector<int>& currentTree);
void ouput(char* fileName);
void deleteBlock(int targetA, vector<int> &currentTree);
void swapBlocks(int targetA, int targetB, vector<int> &currentTree);
void insertBlock(int targetA, int targetB);

void sigHandler(int signum){
	timeFlag = false;
}

int main(int argc, char** argv){
	
	startTime = clock();
	vector<int> currentTree;
	
	vector<int> bestTree;
	
	vector<pair<int, int>> terminals;
	vector<vector<int>> nets;
	alpha = atof(argv[1]);
	parseBlocks(argv[2],  terminals);
	parseNets(argv[3], nets);
	int cnt = 100;
	signal(SIGALRM, sigHandler);
	alarm(270);
	while(cnt-- && timeFlag){
		BTreeInit(currentTree, nets, terminals);
		SA(nets, terminals, currentTree);
	}
	//cout << bestRoot << endl;
	//cout << bestCost << endl;

	ouput(argv[4]);
	
}

void parseBlocks(char* fileName, vector<pair<int, int>>& terminals){
	ifstream finput(fileName, ios::in);
	string useless;

	finput >> useless >> outLineWidth >> outLineHeight;
	finput >> useless >> blockSize;
	finput >> useless >> terminalSize;

	blocks.resize(blockSize+1);
	for(int i = 1; i <= blockSize; i++){
		string name;
		int width, height;
		finput >> name >> width >> height;
		nameMap[name] = i;
		blocks[i] = Block(width, height, name, i);
	}

	terminals.resize(terminalSize);
	for(int i = 0; i < terminalSize; i++){
		string name;
		int width, height;
		finput >> name >> useless;
		nameMap[name] = i+blockSize+1;
		finput >> width >> height;
		terminals[i].first = width;
		terminals[i].second = height;
	}
	finput.close();
}


void parseNets(char* fileName, vector<vector<int>>& nets){
	ifstream finput(fileName, ios::in);
	string useless;
	int netSize;
	finput >> useless >> netSize;
	nets.assign(netSize, {});

	for(int i = 0; i < netSize; i++){
		int netDegree;
		finput >> useless >> netDegree;
		nets[i].assign(netDegree, 0);
		for(int j = 0; j < netDegree; j++){
			string name;
			finput >> name;
			if(nameMap.count(name)){
				nets[i][j] = (nameMap[name]);
			}
			else {
				cerr << "UNKNOWN BLOCK/TERMINAL\n";
				exit(1);
			}
		}
	}
	finput.close();
}

void BTreeInit(vector<int>& currentTree, vector<vector<int>>nets, vector<pair<int, int>> terminals){
	T = 4000000000;
	currentTree.clear();
	currentTree.resize(blockSize + 1);
	iota(currentTree.begin() + 1, currentTree.end(), 1);
	//srand( time(NULL) );
	random_shuffle(currentTree.begin() + 1, currentTree.end());
	leftChild.clear();
	rightChild.clear();
	parent.clear();
	//leftChild.resize(blockSize + 1);
	//rightChild.resize(blockSize + 1);
	//parent.resize(blockSize + 1);
	leftChild.assign(blockSize+1, 0);
	rightChild.assign(blockSize+1, 0);
	parent.assign(blockSize+1, 0);
	
	for(int i = 1; i <= blockSize/2; i ++){
		if(i*2 <= blockSize){
			leftChild[currentTree[i]] = currentTree[i*2];
			parent[currentTree[i*2]] = currentTree[i];
		}else{
			leftChild[currentTree[i]] = 0;
		}
		if(i*2+1 <= blockSize){
			rightChild[currentTree[i]] = currentTree[i*2+1];
			parent[currentTree[i*2+1]] = currentTree[i];
		}else{
			rightChild[currentTree[i]] = 0;
			
		}
	}

	root = currentTree[1];
	reflesh(nets, terminals, currentTree);
}

void SA(vector<vector<int>>nets, vector<pair<int, int>> terminals, vector<int>& currentTree){
	failture = 0;
	int t = 1000000;
	while(t-- && failture < blockSize){
		int oper = rand() % 3 + 1;
		int targetA = (rand() % blockSize) + 1;
		int targetB = (rand() % blockSize) + 1;
		if(oper == 1){
			blocks[targetA].Swap();
			// int temp = blocks[targetA].width;
			// blocks[targetA].width =blocks[targetA].height;
			// blocks[targetA].height = temp;
		}else if(oper <= 2){
			
			if(targetA == targetB) continue;
			deleteBlock(targetA, currentTree);
			insertBlock(targetA, targetB);
		}else if(oper <= 3){
			if(targetA == targetB) continue;
			swapBlocks(targetA, targetB, currentTree);
		}
		reflesh(nets, terminals, currentTree);
	}
}

void updateContour(int current, int preivous, vector<pair<int, int>>& lowerLeft, vector<pair<int, int>>& upperRight, vector<int> &forward, vector<int> &backward){
	if(preivous == 0){
		lowerLeft[current].first = 0;
		upperRight[current].first = blocks[current].width;
		lowerLeft[current].second = 0;
		upperRight[current].second = blocks[current].height;
		forward[current] = backward[current] = 0;
		return;
	}
	if (leftChild[preivous] == current){
		lowerLeft[current].first = upperRight[preivous].first;
		upperRight[current].first = lowerLeft[current].first + blocks[current].width;
		if(forward[preivous] == 0){ //root's leftchild's leftchilds
			lowerLeft[current].second = 0;
			upperRight[current].second = lowerLeft[current].second + blocks[current].height;
			forward[preivous] = current;
			backward[current] = preivous;
			forward[current] = 0;
			return;
		} else {
			backward[current] = preivous;
			forward[current] = forward[preivous];
			backward[forward[current]] = current;
			forward[preivous] = current;
		}
	}else{
		lowerLeft[current].first = lowerLeft[preivous].first;
		upperRight[current].first = lowerLeft[current].first + blocks[current].width;
		if(backward[preivous] == 0){
			backward[current] = 0;
			forward[current] = preivous;
			backward[preivous] = current;
		} else {
			forward[backward[preivous]] = current;
			backward[current] = backward[preivous];
			forward[current] = preivous;
			backward[preivous] = current;
		}

	}
	int maxY = 0;
	int i ;
	for(i = forward[current]; i > 0; i = forward[i]){
		maxY = max(maxY, upperRight[i].second);
		if(upperRight[i].first >= upperRight[current].first){
			if(upperRight[i].first == upperRight[current].first){
				forward[current] = forward[i];
				if(forward[i] == 0){
					backward[forward[i]] = current;
				}
			}else{
				forward[current] = i;
				backward[i] = current;
			}
			break;
			
		}
	}

	if(i == 0){
		forward[current] = 0;
	}
	lowerLeft[current].second = maxY;
	upperRight[current].second = lowerLeft[current].second + blocks[current].height;
}

void dfsUpdate(int curBlock, int preBlock, vector<pair<int, int>>& lowerLeft, vector<pair<int, int>>& upperRight, vector<int> &forward, vector<int> &backward){
	if(curBlock == 0)return;
	updateContour(curBlock, preBlock, lowerLeft, upperRight, forward,  backward);
	dfsUpdate(leftChild[curBlock], curBlock, lowerLeft, upperRight, forward,  backward);
	dfsUpdate(rightChild[curBlock], curBlock, lowerLeft, upperRight, forward,  backward);
}

long countHPWL(vector<vector<int>>nets, vector<pair<int, int>> terminals, vector<pair<int, int>> lowerLeft, vector<pair<int, int>> upperRight){
	long HPWL = 0;

	for(auto net: nets){
		int minX, maxX, minY, maxY;
		minX = minY = INT_MAX;
		maxX = maxY = 0;
		for(auto i: net){
			int centerX, centerY;
			if(i <= blockSize){ //block
				centerX = (lowerLeft[i].first + blocks[i].width/2);
				centerY = (lowerLeft[i].second + blocks[i].height/2);
			}else{  //terminal
				centerX = terminals[i-blockSize-1].first;
				centerY = terminals[i-blockSize-1].second;
			}

			maxX = max(maxX, centerX);
			minX = min(minX, centerX);
			maxY = max(maxY, centerY);
			minY = min(minY, centerY);
		}
		HPWL += (maxX - minX) + (maxY - minY);
	}
	return HPWL;
}

void reflesh(vector<vector<int>>nets, vector<pair<int, int>> terminals, vector<int>& currentTree){
	vector<pair<int, int>>lowerLeft, upperRight;
	vector<int>forward, backward;
	lowerLeft.resize(blockSize+1);
	upperRight.resize(blockSize+1);
	forward.assign(blockSize+1, 0);
	backward.assign(blockSize+1, 0);
	dfsUpdate(root, 0, lowerLeft, upperRight, forward, backward);
	long HPWL = countHPWL(nets,  terminals, lowerLeft, upperRight);
	int maxX = 0, maxY = 0;
	for(int i=1; i <= blockSize; i++){
		maxX = max(maxX, upperRight[i].first);
		maxY = max(maxY, upperRight[i].second);
	}
	int area = maxX * maxY;
	double ratio = double(maxX * outLineHeight) / double(maxY * outLineWidth);
	if (ratio < 1){
		ratio = 1 / ratio;
	}
	double areaCost = area;
	if(maxX > outLineWidth){
		areaCost *= penalty;
	}
	if(maxY > outLineHeight){
		areaCost *= penalty;
	}
	areaCost *= ratio;
	double cost = alpha * area + (1 - alpha) * HPWL;
	bool forceMoveOn = (double)rand() / RAND_MAX < exp(-1*areaCost / T);
	T *= r;
	if(areaCost <= preCost || forceMoveOn){
		//currentTree = preivousTree;
		preParent = parent;
		preLeftChild = leftChild;
		preRightChild = rightChild;
		preBlocks = blocks;
		preRoot = root;
		preCost = areaCost;
		failture = 0;
	}else{
		//preivousTree = currentTree;
		parent = preParent;
		leftChild = preLeftChild;
		rightChild = preRightChild;
		blocks = preBlocks;
		root = preRoot;
		failture ++ ;
		return;
	}
	
	
	if(cost <= bestCost && maxX <= outLineWidth*1 && maxY <= outLineHeight*1){
		bestCost = cost;
		bestArea = area;
		bestWidth = maxX;
		bestHeight = maxY;
		bestHPWL = HPWL;
		bestRoot = root;
		//bestBlocks = blocks;
		bestBlocksPlacement.resize(blockSize+1);
		for(int i = 1; i <= blockSize; i++){
			bestBlocksPlacement[i] = (vector<int>({lowerLeft[i].first, lowerLeft[i].second, upperRight[i].first, upperRight[i].second}));
		}
		
	}
	return;
}
void ouput(char* fileName){
	fstream fouput;
	fouput.open(fileName, ios::out);
	endTime = clock();
	fouput << bestCost << '\n' << bestHPWL << '\n' << bestArea << '\n' << bestWidth << ' ' << bestHeight << '\n' << (endTime-startTime)/CLOCKS_PER_SEC << '\n';
	
	for(int i= 1; i <= blockSize; i ++){
		
		fouput << blocks[i].name << ' ';
		for(auto j:bestBlocksPlacement[i] ){
			fouput << j << ' ';
		} 
		fouput << '\n';
	}
	fouput.close();
}

void swapBlocks(int targetA, int targetB, vector<int> &currentTree){

	if(parent[targetA] != targetB && parent[targetB] != targetA){
		if(root == targetA){
			root = targetB;
		}else if (root == targetB){
			root = targetA;
		}
		if(leftChild[targetA] > 0){
			parent[leftChild[targetA]] = targetB;
		}
		if(leftChild[targetB] > 0){
			parent[leftChild[targetB]] = targetA;
		}
		if(rightChild[targetA] > 0){
			parent[rightChild[targetA]] = targetB;
		}
		if(rightChild[targetB] > 0){
			parent[rightChild[targetB]] = targetA;
		}
		
		if(parent[targetA] > 0){
			if(leftChild[parent[targetA]] == targetA){
				leftChild[parent[targetA]] = targetB;
			}else if (rightChild[parent[targetA]] == targetA){
				rightChild[parent[targetA]] = targetB;
			}
		}

		if(parent[targetB] > 0){
			if(leftChild[parent[targetB]] == targetB){
				leftChild[parent[targetB]] = targetA;
			}else if (rightChild[parent[targetB]] == targetB){
				rightChild[parent[targetB]] = targetA;
			}
		}
		// int temp = leftChild[targetA];
		// leftChild[targetA] = leftChild[targetB];
		// leftChild[targetB] = temp;
		// temp = rightChild[targetA];
		// rightChild[targetA] =rightChild[targetB];
		// rightChild[targetB] = temp;
		// temp = parent[targetA];
		// parent[targetA] = parent[targetB];
		// parent[targetB] = temp;
		std::swap(leftChild[targetA], leftChild[targetB]);
		std::swap(rightChild[targetA], rightChild[targetB]);
		std::swap(parent[targetA], parent[targetB]);

	}else{
		if(parent[targetA] == targetB){
			std::swap(targetA, targetB);
		}

		if(root == targetA){
			root = targetB;
		}else if (root == targetB){
			root = targetA;
		}
		if(leftChild[targetA] == targetB){
			if(rightChild[targetA] > 0){
				parent[rightChild[targetA]] = targetB;
			}
			if(rightChild[targetB] > 0){
				parent[rightChild[targetB]] = targetA;
			}
			// int temp = rightChild[targetA];
			// rightChild[targetA] = rightChild[targetB];
			// rightChild[targetB] = temp;
			std::swap(rightChild[targetA], rightChild[targetB]);
			if(leftChild[targetB] > 0){
				parent[leftChild[targetB]] = targetA;
			}
			leftChild[targetA] = leftChild[targetB];
			leftChild[targetB] = targetA;
		}else if(rightChild[targetA] == targetB){
			if(leftChild[targetA] > 0){
				parent[leftChild[targetA]] = targetB;
			}
			if(leftChild[targetB] > 0){
				parent[leftChild[targetB]] = targetA;
			}
			// int temp = leftChild[targetA];
			// leftChild[targetA] = leftChild[targetB];
			// leftChild[targetB] = temp;
			std::swap(leftChild[targetA], leftChild[targetB]);
			if(rightChild[targetB] > 0){
				parent[rightChild[targetB]] = targetA;
			}
			rightChild[targetA] = rightChild[targetB];
			rightChild[targetB] = targetA;
		}
		if(parent[targetA] > 0){
			if(leftChild[parent[targetA]] == targetA){
				leftChild[parent[targetA]] = targetB;
				parent[targetB] = parent[targetA];
				parent[targetA] = targetB;
			}else if(rightChild[parent[targetA]] == targetA){
				rightChild[parent[targetA]] = targetB;
				parent[targetB] = parent[targetA];
				parent[targetA] = targetB;
			}
		}else{
			parent[targetB] = parent[targetA];
			parent[targetA] = targetB;
		}

	}
}
void deleteBlock(int targetA, vector<int> &currentTree){
	if(blockSize == 1)return;
	if(leftChild[targetA] == 0 && rightChild[targetA] == 0){
		if(rightChild[parent[targetA]] == targetA){
			rightChild[parent[targetA]] = 0;
		}else if(leftChild[parent[targetA]] == targetA){
			leftChild[parent[targetA]] = 0;
		}
	}else{
		if(leftChild[targetA] > 0 && rightChild[targetA] > 0){
			if(rand() % 2){
				swapBlocks(targetA, leftChild[targetA], currentTree);
				deleteBlock(targetA, currentTree);
			}else{
				swapBlocks(targetA, rightChild[targetA], currentTree);
				deleteBlock(targetA, currentTree);
			}
		}else{
			if(leftChild[targetA] > 0){
				swapBlocks(targetA, leftChild[targetA], currentTree);
				deleteBlock(targetA, currentTree);
			}else{
				swapBlocks(targetA, rightChild[targetA], currentTree);
				deleteBlock(targetA, currentTree);
			}
		}
		
	}

}
void insertBlock(int targetA, int targetB){
	parent[targetA] = targetB;
	if(rand()% 2){
		leftChild[targetA] = leftChild[targetB];
		if(leftChild[targetA] > 0){
			parent[leftChild[targetA]] = targetA;
		}
		leftChild[targetB] = targetA;
		rightChild[targetA] = 0;
	}else{
		rightChild[targetA] = rightChild[targetB];
		if(rightChild[targetA] > 0){
			parent[rightChild[targetA]] = targetA;
		}
		rightChild[targetB] = targetA;
		leftChild[targetA] = 0;
	}
}