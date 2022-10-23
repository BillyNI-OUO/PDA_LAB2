#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>
#include <random>
#include <climits>
#include <float.h>
#include "./src/block.cpp"

using namespace std;
int outLineWidth, outLineHeight;
int blockSize, terminalSize;
unordered_map<string, int> nameMap;
vector<int> leftChild, rightChild, parent;
vector<int> preivousTree;
vector<int> preLeftChild, preRightChild, preParent;
vector<vector<int>>bestBlocksPlacement;
vector<Block> preBlocks;

double bestCost = DBL_MAX;
int bestArea = -1;
int bestWidth = -1;
int bestHeight = -1;
int bestHPWL = -1;
double alpha = 0.5;
int failture  = 0;

void parseBlocks(char* fileName, vector<Block>& blocks, vector<pair<int, int>>& terminals);
void parseNets(char* fileName, vector<vector<int>>& nets);
void BTreeInit(vector<int>& currentTree, vector<Block>& blocks, vector<vector<int>>nets, vector<pair<int, int>> terminals);
void SA(vector<Block>& blocks, int root, vector<vector<int>>nets, vector<pair<int, int>> terminals, vector<int>& currentTree);
void updateContour(int current, int preivous, vector<pair<int, int>>& lowerLeft, vector<pair<int, int>>& upperRight, vector<int> &forward, vector<Block> blocks, vector<int> &backward);
void dfsUpdate(int curBlock, int preBlock, vector<pair<int, int>>& lowerLeft, vector<pair<int, int>>& upperRight, vector<int> &forward, vector<Block> blocks, vector<int> &backward);
long countHPWL(vector<vector<int>>nets, vector<Block> blocks, vector<pair<int, int>> terminals, vector<pair<int, int>> lowerLeft, vector<pair<int, int>> upperRight);
void reflesh(int root, vector<Block> blocks, vector<vector<int>>nets, vector<pair<int, int>> terminals, vector<int>& currentTree);


int main(int argc, char** argv){
	vector<int> currentTree;
	
	vector<int> bestTree;
	vector<Block> blocks;
	vector<pair<int, int>> terminals;
	vector<vector<int>> nets;

	parseBlocks(argv[1], blocks, terminals);
	parseNets(argv[2], nets);

	int cnt = 1;
	while(cnt--){
		BTreeInit(currentTree, blocks, nets, terminals);
		SA(blocks, currentTree[1], nets, terminals, currentTree);
	}
	cout << currentTree[1] << endl;
	cout << bestCost << endl;
	
}

void parseBlocks(char* fileName, vector<Block>& blocks, vector<pair<int, int>>& terminals){
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
		finput >> name >> useless;
		nameMap[name] = i+blockSize+1;
		finput >> terminals[i].first >> terminals[i].second;
	}
	finput.close();
}


void parseNets(char* fileName, vector<vector<int>>& nets){
	ifstream finput(fileName, ios::in);
	string useless;
	int netSize;
	finput >> useless >> netSize;
	nets.resize(netSize);

	for(int i = 0; i < netSize; i++){
		int netDegree;
		finput >> useless >> netDegree;
		for(int j = 0; j < netDegree; j++){
			string name;
			finput >> name;
			if(nameMap.count(name)){
				nets[i].push_back(nameMap[name]);
			}
			else {
				
				cerr << "UNKNOWN BLOCK/TERMINAL\n";
				exit(1);
			}
		}
	}
}

void BTreeInit(vector<int>& currentTree, vector<Block>& blocks, vector<vector<int>>nets, vector<pair<int, int>> terminals){
	currentTree.clear();
	currentTree.resize(blockSize + 1);
	iota(currentTree.begin() + 1, currentTree.end(), 1);
	srand( time(NULL) );
	random_shuffle(currentTree.begin() + 1, currentTree.end());
	
	leftChild.resize(blockSize + 1);
	rightChild.resize(blockSize + 1);
	parent.resize(blockSize + 1);
	
	for(int i = 1; i <= blockSize; i ++){
		if(i*2 <= blockSize){
			leftChild[currentTree[i]] = currentTree[i*2];
		}else{
			leftChild[currentTree[i]] = 0;
		}
		if(i*2+1 <= blockSize){
			rightChild[currentTree[i]] = currentTree[i*2+1];
		}else{
			rightChild[currentTree[i]] = 0;
		}
		if(i != 1){
			parent[currentTree[i]] = currentTree[i/2];
		}else{
			parent[currentTree[i]] = 0;
		}
	}


	reflesh(currentTree[1], blocks, nets, terminals, currentTree);
}

void SA(vector<Block>& blocks, int root, vector<vector<int>>nets, vector<pair<int, int>> terminals, vector<int>& currentTree){
	int failCnt = 0;
	int t = 100000;
	while(t-- && failCnt < blockSize){
		
		//Op1: Rotate a module.
		//[Op2: Flip a module.] 
		//Op3: Move a module to another place.
		// Op4: Swap two modules.
		//cout << pre_chips_width << " " << pre_chips_height << " ";
		
		int oper = rand() % 3;
		int tar = (rand() % blockSize) + 1;
		if(oper < 1){
			blocks[tar].Swap();
		}else if(oper < 2){
			//could be 0 to insert to root :)
			//int tar2 = rand() % (block_size + 1);
			int tar2 = (rand() % blockSize) + 1;
			if(tar == tar2) continue;
			//node_erase(tar);
			//node_insert(tar, tar2);
		}else if(oper < 3){
			int tar2 = (rand() % blockSize) + 1;
			if(tar == tar2) continue;
			//node_swap(tar, tar2);
		}
		reflesh(root, blocks, nets, terminals, currentTree);
	}
}

void updateContour(int current, int preivous, vector<pair<int, int>>& lowerLeft, vector<pair<int, int>>& upperRight, vector<int> &forward, vector<Block> blocks, vector<int> &backward){
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
			upperRight[current].second = blocks[current].height;
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
		lowerLeft[current].first = lowerLeft[preivous].second;
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


	int i = forward[current];
	while(i != 0){
		lowerLeft[i].second = max(lowerLeft[current].second, upperRight[i].second);
		
		if(upperRight[i].first == upperRight[current].first) {
			forward[current] = forward[i];
			if(forward[i] == 0){
				backward[forward[i]] = current;
			}
			break;
		}else if (upperRight[i].first > upperRight[current].first){
			forward[current] = i;
			backward[i] = current;
			break;
		}else {
			i = forward[i];
		}
	}

	if(i == 0){
		forward[current] = 0;
	}
	upperRight[current].second = upperRight[current].second + blocks[current].height;
}

void dfsUpdate(int curBlock, int preBlock, vector<pair<int, int>>& lowerLeft, vector<pair<int, int>>& upperRight, vector<int> &forward, vector<Block> blocks, vector<int> &backward){
	if(curBlock == 0)return;
	updateContour(curBlock, preBlock, lowerLeft, upperRight, forward, blocks,  backward);
	dfsUpdate(leftChild[curBlock], curBlock, lowerLeft, upperRight, forward, blocks,  backward);
	dfsUpdate(rightChild[curBlock], curBlock, lowerLeft, upperRight, forward, blocks,  backward);
}

long countHPWL(vector<vector<int>>nets, vector<Block> blocks, vector<pair<int, int>> terminals, vector<pair<int, int>> lowerLeft, vector<pair<int, int>> upperRight){
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

void reflesh(int root, vector<Block> blocks, vector<vector<int>>nets, vector<pair<int, int>> terminals, vector<int>& currentTree){
	vector<pair<int, int>>lowerLeft, upperRight;
	vector<int>forward, backward;
	lowerLeft.resize(blockSize+1);
	upperRight.resize(blockSize+1);
	forward.assign(blockSize+1, 0);
	backward.assign(blockSize+1, 0);
	
	dfsUpdate(root, 0, lowerLeft, upperRight, forward, blocks, backward);

	long HPWL = countHPWL(nets, blocks, terminals, lowerLeft, upperRight);
	int maxX = 0, maxY = 0;
	for(int i=1; i <= blockSize; i++){
		maxX = max(maxX, upperRight[i].first);
		maxY = max(maxY, upperRight[i].second);
	}
	long area = (long)maxX * maxY;
	double cost = alpha * area + (1 - alpha) * HPWL;

	if(maxX > outLineWidth*2 || maxY > outLineHeight*2){
		currentTree = preivousTree;
		parent = preParent;
		leftChild = preLeftChild;
		rightChild = preLeftChild;
		blocks = preBlocks;
		failture ++ ;
		return;
	}else{
		preivousTree = currentTree;
		preParent = parent;
		preLeftChild = leftChild;
		preLeftChild = rightChild;
		preBlocks = blocks;
	}

	if(cost < bestCost){
		bestCost = cost;
		bestArea = area;
		bestWidth = maxX;
		bestHeight = maxY;
		bestHPWL = HPWL;

		bestBlocksPlacement.resize(blockSize + 1);
		for(int i = 1; i <= blockSize; i++){
			bestBlocksPlacement[i] = vector<int>({lowerLeft[i].first, lowerLeft[i].second, upperRight[i].first, upperRight[i].second});
		}
	}
	return;
}
