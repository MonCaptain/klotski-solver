/* compute optimal solutions for sliding block puzzle. */
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <cstdlib>   /* for atexit() */
#include <cassert>
#include <string>
using std::string;
#include <unordered_map>
using std::unordered_map;
#include <iostream> 
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <algorithm>
using std::swap;
#include <queue>
using std::queue;
/* SDL reference: https://wiki.libsdl.org/CategoryAPI */

/* initial size; will be set to screen size after window creation. */
int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;
int fcount = 0;
int mousestate = 0;
SDL_Point lastm = {0,0}; /* last mouse coords */
SDL_Rect bframe; /* bounding rectangle of board */
static const int ep = 2; /* epsilon offset from grid lines */

bool init(); /* setup SDL */
void initBlocks();

//Full screen or windowed:
// #define FULLSCREEN_FLAG SDL_WINDOW_FULLSCREEN_DESKTOP
#define FULLSCREEN_FLAG 0

/* NOTE: ssq == "small square", lsq == "large square" */
enum bType {hor,ver,ssq,lsq};
struct block {
	SDL_Rect R; /* screen coords + dimensions */
	bType type; /* shape + orientation */
	bType id;   /* block id */
	/* TODO: you might want to add other useful information to
	 * this struct, like where it is attached on the board.
	 * (Alternatively, you could just compute this from R.x and R.y,
	 * but it might be convenient to store it directly.) */
	void rotate() /* rotate rectangular pieces */
	{
		if (type != hor && type != ver) return;
		type = (type==hor)?ver:hor;
		swap(R.w,R.h);
	}
};

#define NBLOCKS 10
block B[NBLOCKS];
block* dragged = NULL;

block* findBlock(int x, int y);
void close(); /* call this at end of main loop to free SDL resources */
SDL_Window* gWindow = 0; /* main window */
SDL_Renderer* gRenderer = 0;

// safe to ignore
bool init()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL_Init failed.  Error: %s\n", SDL_GetError());
		return false;
	}
	/* NOTE: take this out if you have issues, say in a virtualized
	 * environment: */
	if(!SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1")) {
		printf("Warning: vsync hint didn't work.\n");
	}
	/* create main window */
	gWindow = SDL_CreateWindow("Sliding block puzzle solver",
								SDL_WINDOWPOS_UNDEFINED,
								SDL_WINDOWPOS_UNDEFINED,
								SCREEN_WIDTH, SCREEN_HEIGHT,
								SDL_WINDOW_SHOWN|FULLSCREEN_FLAG);
	if(!gWindow) {
		printf("Failed to create main window. SDL Error: %s\n", SDL_GetError());
		return false;
	}
	/* set width and height */
	SDL_GetWindowSize(gWindow, &SCREEN_WIDTH, &SCREEN_HEIGHT);
	/* setup renderer with frame-sync'd drawing: */
	gRenderer = SDL_CreateRenderer(gWindow, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if(!gRenderer) {
		printf("Failed to create renderer. SDL Error: %s\n", SDL_GetError());
		return false;
	}
	SDL_SetRenderDrawBlendMode(gRenderer,SDL_BLENDMODE_BLEND);

	initBlocks();
	return true;
}

void printV(vector<vector<string> >& grid){
    for (size_t i = 0; i<grid.size(); i++){
        for (size_t j = 0; j < grid[i].size(); j++){
            cout << grid[i][j] << "\t"; 
        }
        cout << endl;
    }
    cout << endl;
}

//some configurations:
// 117 step solution:
// vector<vector <string>> grid  = {{"1", "b", "b", "2"},
//                                  {"1", "b", "b", "2"},
//                                  {"3", "5", "5", "4"},
//                                  {"3", "6", "7", "4"},
//                                  {"8", "e", "e", "9"}};

// 126 step solution:
vector<vector <string>> grid  = {{"1", "b", "b", "2"},
                                 {"1", "b", "b", "2"},
                                 {"5", "5", "3", "4"},
                                 {"7", "6", "3", "4"},
                                 {"8", "e", "e", "9"}};

// 18 step solution:
// vector<vector <string>> grid  = {{"e", "1", "4", "2"},
//                                  {"3", "1", "4", "2"},
//                                  {"3", "7", "b", "b"},
//                                  {"e", "6", "b", "b"},
//                                  {"5", "5", "8", "9"}};




//TODO: find the winning configuration using BFS
void right(string& sample, int a, int b){

    //making sure that the rectangle block that was not moved already (in other words sample[b] is not the bottom half)
        /* Detailed reasoning: since we're scanning for empty space from the left to the right row by row, 
        we need to only make sure that the block is not a rectangle by checking if its value above is the same
        otherwise we'd swap half the block*/
    if (sample[b] != sample[b-4]){

    //checks if sample[b] is the upper half of a rectangle block to the right of sample[a]
        if (sample[a] == sample[a+4] && sample[b] == sample[b+4] && b+4<=19){
            swap(sample[a], sample[b]);
            swap(sample[a+4], sample[b+4]);

            if (sample[a] == sample[b+1] && b+1 <= 19){
                //+1 since big square is to the right
                swap(sample[b], sample[b+1]);
                swap(sample[b+4], sample[b+1+4]);                
            }

        }
        //checking that it is a horizontal block and not a big square
        else if (sample[b]==sample[b+1] && sample[b]!=sample[b+4] && b+1<=19) {
            swap(sample[a], sample[b+1]);
        }
        //makes sure that we're sliding a small square (we're checking if sample[b+4] is identical to the right cell)
        else if (sample[b]!=sample[b+4]) swap(sample[a], sample[b]); 
    }
}

void left(string& sample, int a, int b){

    //making sure that we're not moving a rectangle block that has been already moved
    if (sample[b] != sample[b-4]){
        //checks if sample[b] is the upper half of a rectangle block to the left of sample[a] and if there;s space to move it
        if (sample[a] == sample[a+4] && sample[b] == sample[b+4] && b+4<= 19){
            swap(sample[a], sample[b]);
            swap(sample[a+4], sample[b+4]);
        //check if the block has a width of 2, then make some extra swaps
            if (sample[a] == sample[b-1] && b-1 >= 0){
                swap(sample[b], sample[b-1]);
                swap(sample[b+4], sample[b-1+4]);
            }
        }
        //checks if there's a horizontal block to the left of sample[a] 
        else if (sample[b] == sample[b-1] && sample[b]!=sample[b+4]  && b-1>=0) 
            swap(sample[a], sample[b-1]);

        //makes sure that we're sliding a small square (we're checking if sample[b+4] is identical to left cell)
        else if (sample[b]!= sample[b+4]) swap(sample[a], sample[b]);
    }
}

void above(string& sample, int a, int b){

    /*checks for horizontal block above the empty space 
    (only to check the right of the block and the empty space to check for width) */
 if (sample[b] != sample[b-1] && b-1>=0){
     
     //checks if there's space to move a horizontal block if there's any
    if (sample[a]==sample[a+1] && sample[b] == sample[b+1]){
        swap(sample[a], sample[b]);     //swapping empty space with block
        swap(sample[a+1], sample[b+1]); //swapping the right empty space with the other half of the block
        
        //check if its a big square, do some extra swaps to the other half of the block
        if (sample[a] == sample[b-4] && b-4 >= 0){
            swap(sample[b], sample[b-4]);
            swap(sample[b+1], sample[b+1-4]); 
        }
    }
    //checks for a vertical block and makes sure that it is not a big square
    else if (sample[b] == sample[b-4] && sample[b]!=sample[b+1] && b-4 >= 0){
        swap(sample[a], sample[b-4]);
    }
    ////makes sure that we're sliding a small square (we're checking if sample[b+1] is identical to the cell above)
    else if (sample[b]!=sample[b+1]) swap(sample[a], sample[b]);
 }
}

void below(string& sample, int a, int b){

    //ensures that the block is not part of a rectangle that we have checked previously
    if (sample[b] != sample[b-1]){
        
    /*checks for horizontal block above the empty space 
    (only to check the right of the block and the empty space to check for width) */
        if (sample[a]==sample[a+1] && sample[b] == sample[b+1]){
        swap(sample[a], sample[b]);     //swapping empty space with block directly below
        swap(sample[a+1], sample[b+1]); //swapping the right empty space with the other half of the block
        
        //check if its a big square, do some extra swaps to the other half of the block if it exists
            if (sample[a] == sample[b+4] && b+4 <= 19){
                swap(sample[b], sample[b+4]);
                swap(sample[b+1], sample[b+1+4]);
            }
        }
        /*checks for vertical block:
        the idea is to swap empty space with the cell 2 units below
        instead of two swaps*/
        else if (sample[b] == sample[b+4] && sample[b]!=sample[b+1] && b+4 <= 19){
            swap(sample[a], sample[b+4]);
        }
        //small square:
        else if (sample[b]!=sample[b+1]) swap(sample[a], sample[b]);
    }
}	

vector<string> getNeighbors(string& vertex){
    
    vector<string> n;
    for (size_t i=0; i < vertex.length(); i++){
        // maximum number of neighbors for e is 4

        if (vertex[i] == 'e'){
            // case 1: right of empty space
            if (vertex[i] == 'e' && (i)%4!=3 && vertex[i+1] != 'e'){    
                string sample = vertex;
                right(sample, i, i+1);
                n.push_back(sample);
            }
            // case 2: left of empty space 
            if (vertex[i] == 'e' && (i)%4!=0 && vertex[i-1] != 'e'){
                string sample = vertex;
                left(sample, i, i-1);
                n.push_back(sample);
            }
            // case 3: above empty space
            if (vertex[i] == 'e' && i-4 >= 0 && vertex[i-4] != 'e'){
                string sample = vertex;
                above(sample, i, i-4);
                n.push_back(sample);
            }
            // case 4: below empty space
            if (vertex[i] == 'e' && i+4 <= 19 && vertex[i+4] != 'e'){
                string sample = vertex;
                below(sample, i, i+4);
                n.push_back(sample);
            }
        } 
    }
            return n;
}

vector<string> getPath(string final_config, unordered_map <string,string>& came_from){

    vector<string> path;
    string temp=final_config;
	path.push_back(temp);
    while(came_from[temp]!="None"){
        path.push_back(came_from[temp]);
        temp = came_from[temp];
    }
    cout << "size is " << path.size() << endl;
    return path;
}

bool won(string& s){

        if (s[13] == 'b' && s[14] == 'b' && s[17] == 'b' && s [18] =='b'){
            cout << "solution found:" << s << endl;
            return true;
        }

    return false;
}

string vector_to_string(vector<vector<string> >& v);

vector<string> bfs(vector<vector<string> >& grid){
    string sample = vector_to_string(grid);
    
    queue<string> frontier;
    frontier.push(sample);
    // map to store board configurations
    unordered_map <string,string> came_from = {{sample, "None"}};

    while (frontier.empty()==false){
        string currentS = frontier.front();
        frontier.pop();

        vector<string> neighbors = getNeighbors(currentS); //getting possible moves to make from the current configuration
        
        for (size_t i=0; i<neighbors.size(); i++){ //iterating through the available moves we can make
            
            //checks if we found the winning configuration
            if (won(neighbors[i])==true){
                came_from.insert({neighbors[i], currentS});
                 return getPath(neighbors[i], came_from);   //getting all moves for the solution
				//  return;
            }
            /*if neighbors[i] is a configuration that we see for the first time, add it to the queue 
            and keep track of its parent */
            if (came_from.find(neighbors[i]) == came_from.end()){
                frontier.push(neighbors[i]);
                came_from.insert({neighbors[i], currentS});
            }
        }
    }
    // while loop terminates if there's no path to winning
    cout << "no solution found\n";
	vector<string> s;
	return s;
}

const int WIDTH = 4;
const int HEIGHT = 5;

vector<vector<string> > string_to_vector(string& s){
    vector<vector<string>> v;
    vector<string> temp(WIDTH);
    for (size_t i = 1; i <= s.length(); i++){
        temp[(i-1)%WIDTH] = s[i-1];
        if (i%WIDTH == 0)v.push_back(temp);

    }

    return v;
}

string vector_to_string(vector<vector<string> >& v){
    string s = "";
       for (size_t i = 0; i<v.size(); i++){
        for (size_t j = 0; j < v[i].size(); j++){
            s += v[i][j];
        }
    }
    return s;
}


/* TODO: you'll probably want a function that takes a state / configuration
 * and arranges the blocks in accord.  This will be useful for stepping
 * through a solution.  Be careful to ensure your underlying representation
 * stays in sync with what's drawn on the screen... */
void initBlocks();

void moveBlocks(string state = ""){
	//ep = is a single line between units on the board
	//u = side length of the square on the board
	int& W = SCREEN_WIDTH;
	int& H = SCREEN_HEIGHT;
	int h = H*3/4; 		// UNSURE: height of a single unit block relative to the window/screen 
	int w = 4*h/5;		// UNSURE: width of a single unit block relative to the window/screen 
	int u = h/5-2*ep;	// 
	int mw = (W-w)/2;   // width of the board
	int mh = (H-h)/2;   // height of the board
	
	// state = "bb11bb223554367489ee";
	// state =  "1bb21bb2355436748ee9";
	if (state.empty()) state = vector_to_string(grid);
	unordered_map<char, char> map; 	 //to keep track of rectangles
	
	// 5-8 squares
	int j=5;
	for (size_t i = 0; i < state.length(), j < 9; i++ ){
		
		
			if (map.find(state[i]) == map.end() && state[i]!='e'){
				
				//rectangle or big square
            	if (state[i] == state[i+1] || (state[i] == state[i+4] && i+4 < 20)){
                map[state[i]]=state[i];  
            	}
            	else if (state[i] != state[i+1] && state[i] != state[i+4]) {
					B[j].R.x = mw+ep + (i%4)*(u+2*ep);	//to find width we mod 4
					B[j].R.y = mh+ep + (i/4)*(u+2*ep);	//to find height we divide by 4 
					j++;

				}
        	}	
	}

	map.clear();
	j = 0;
	for (size_t i=0; i<state.length(); i++){
		
		if (map.find(state[i]) == map.end() && state[i]!='e')
        {
			//big square
			if (state[i] == state[i+1] && state[i] == state[i+4] && i+4 < 20){
				map[state[i]]=state[i];
				B[9].R.x = mw+ep + (i%4)*(u+2*ep);
				B[9].R.y = mh+ep + (i/4)*(u+2*ep);
			}
			//horizontal rectangle
            else if (state[i] == state[i+1]){
                map[state[i]]=state[i];
				//postition:
				B[j].R.x = mw+ep + (i%4)*(u+2*ep);
				B[j].R.y = mh+ep + (i/4)*(u+2*ep);
				//orientation:
				B[j].R.w = 2*(u+ep);
				B[j].R.h = u;
				j++;
            }
			//vertical rectangle
			else if(state[i] == state[i+4] && i+4 < 20){
				map[state[i]]=state[i];
				//position:
				B[j].R.x = mw+ep + (i%4)*(u+2*ep);
				B[j].R.y = mh+ep + (i/4)*(u+2*ep);
				//orientation:
				B[j].R.w = u;
				B[j].R.h = 2*(u+ep);
				j++;
			}
		}
	}
}

void scanBlocks() {
	// x index = (b->R.x - 850)/216 ----> will give a number between 0-3
	// y index = (b->R.y - 182)/216 ----> will give a number between 0-4
	#if 0
	for (size_t i = 5; i < 9; i++){
		// int x = (B[i].R.x -850)/216;
		// int y = (B[i].R.y-182)/216;
		cout << B[i].R.x << "\t" << B[i].R.y << endl;
		// grid[x][y] = i;
	}
#endif


	printV(grid);

}

bool checkBlocks(){
	int count = 0;
	for (size_t i = 0; i<grid.size(); i++){
		for (size_t j = 0; j < grid[i].size(); j++){
            if (grid[i][j] == "e") count++;
        }
	}
	if (count == 2) return true;
	else return false;
}

//safe to ignore
void initBlocks()
{
	//ep = is a single line between units on the board
	//u = side length of the square on the board
	int& W = SCREEN_WIDTH;
	int& H = SCREEN_HEIGHT;
	int h = H*3/4; 		// UNSURE: height of a single unit block relative to the window/screen 
	int w = 4*h/5;		// UNSURE: width of a single unit block relative to the window/screen 
	int u = h/5-2*ep;	// 
	int mw = (W-w)/2;   // width of the board
	int mh = (H-h)/2;   // height of the board

	/* setup bounding rectangle of the board: */
	bframe.x = (W-w)/2;
	bframe.y = (H-h)/2;
	bframe.w = w;
	bframe.h = h;

	/* NOTE: there is a tacit assumption that should probably be
	 * made explicit: blocks 0-4 are the rectangles, 5-8 are small
	 * squares, and 9 is the big square.  This is assumed by the
	 * drawBlocks function below. */
// Professors Code:

/*R.x and R.y are positions of the blocks on the window
R.h and R.w is the size of the blocks */
	for (size_t i = 0; i < 5; i++) {
		B[i].R.x = (mw-2*u)/2;
		B[i].R.y = mh + (i+1)*(u/5) + i*u;
		B[i].R.w = 2*(u+ep);
		B[i].R.h = u;
		B[i].type = hor;
		// B[i].id = i+1;
	}
	/*x = mw+ep and y = mh+ep sets a block to the top left corner of the board
	changing coefficient of (u+2*ep) by an integer changes where the block is located on the board*/
	B[4].R.x = mw+ep;
	B[4].R.y = mh+ep;
	
	//multipying u by an integer n increases width/length of the block by the value of n
	B[4].R.w = 2*(u+ep);
	B[4].R.h = u;
	B[4].type = hor;

	/* small squares */
	for (size_t i = 0; i < 4; i++) {
		B[i+5].R.x = (W+w)/2 + (mw-2*u)/2 + (i%2)*(u+u/5);
		B[i+5].R.y = mh + ((i/2)+1)*(u/5) + (i/2)*u;
		B[i+5].R.w = u;
		B[i+5].R.h = u;
		B[i+5].type = ssq;
	}
	B[9].R.x = B[5].R.x + u/10;
	B[9].R.y = B[7].R.y + u + 2*u/5;
	B[9].R.w = 2*(u+ep);
	B[9].R.h = 2*(u+ep);
	B[9].type = lsq;
}

// safe to ignore
void drawBlocks()
{
	/* rectangles */
	SDL_SetRenderDrawColor(gRenderer, 0x43, 0x4c, 0x5e, 0xff);
	for (size_t i = 0; i < 5; i++) {
		SDL_RenderFillRect(gRenderer,&B[i].R);
	}
	/* small squares */
	SDL_SetRenderDrawColor(gRenderer, 0x5e, 0x81, 0xac, 0xff);
	for (size_t i = 5; i < 9; i++) {
		SDL_RenderFillRect(gRenderer,&B[i].R);
	}
	/* large square */
	SDL_SetRenderDrawColor(gRenderer, 0xa3, 0xbe, 0x8c, 0xff);
	SDL_RenderFillRect(gRenderer,&B[9].R);
}

// safe to ignore for now
/* return a block containing (x,y), or NULL if none exists. */
block* findBlock(int x, int y)
{
	/* NOTE: we go backwards to be compatible with z-order */
	for (int i = NBLOCKS-1; i >= 0; i--) {
		if (B[i].R.x <= x && x <= B[i].R.x + B[i].R.w &&
				B[i].R.y <= y && y <= B[i].R.y + B[i].R.h)
			return (B+i);
	}
	return NULL;
}

// safe to ignore
void close()
{
	SDL_DestroyRenderer(gRenderer); gRenderer = NULL;
	SDL_DestroyWindow(gWindow); gWindow = NULL;
	SDL_Quit();
}

// safe to ignore
void render()
{
	/* draw entire screen to be black: */
	SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xff);
	SDL_RenderClear(gRenderer);

	/* first, draw the frame: */
	int& W = SCREEN_WIDTH;
	int& H = SCREEN_HEIGHT;
	int w = bframe.w;
	int h = bframe.h;
	SDL_SetRenderDrawColor(gRenderer, 0x39, 0x39, 0x39, 0xff);
	SDL_RenderDrawRect(gRenderer, &bframe);
	/* make a double frame */
	SDL_Rect rframe(bframe);
	int e = 3;
	rframe.x -= e; 
	rframe.y -= e;
	rframe.w += 2*e;
	rframe.h += 2*e;
	SDL_RenderDrawRect(gRenderer, &rframe);

	/* draw some grid lines: */
	SDL_Point p1,p2;
	SDL_SetRenderDrawColor(gRenderer, 0x19, 0x19, 0x1a, 0xff);
	/* vertical */
	p1.x = (W-w)/2;
	p1.y = (H-h)/2;
	p2.x = p1.x;
	p2.y = p1.y + h;
	for (size_t i = 1; i < 4; i++) {
		p1.x += w/4;
		p2.x += w/4;
		SDL_RenderDrawLine(gRenderer,p1.x,p1.y,p2.x,p2.y);
	}
	/* horizontal */
	p1.x = (W-w)/2;
	p1.y = (H-h)/2;
	p2.x = p1.x + w;
	p2.y = p1.y;
	for (size_t i = 1; i < 5; i++) {
		p1.y += h/5;
		p2.y += h/5;
		SDL_RenderDrawLine(gRenderer,p1.x,p1.y,p2.x,p2.y);
	}
	SDL_SetRenderDrawColor(gRenderer, 0xd8, 0xde, 0xe9, 0x7f);
	SDL_Rect goal = {bframe.x + w/4 + ep, bframe.y + 3*h/5 + ep,
	                 w/2 - 2*ep, 2*h/5 - 2*ep};
	SDL_RenderDrawRect(gRenderer,&goal);

	/* now iterate through and draw the blocks */
	drawBlocks();
	/* finally render contents on screen, which should happen once every
	 * vsync for the display */
	SDL_RenderPresent(gRenderer);
}
//ignore for now
void snap(block* b)
{

	/* TODO: once you have established a representation for configurations,
	 * you should update this function to make sure the configuration is
	 * updated when blocks are placed on the board, or taken off.  */
	assert(b != NULL);
	/* upper left of grid element (i,j) will be at
	 * bframe.{x,y} + (j*bframe.w/4,i*bframe.h/5) */
	/* translate the corner of the bounding box of the board to (0,0). */
	int x = b->R.x - bframe.x;
	int y = b->R.y - bframe.y;
	int uw = bframe.w/4;
	int uh = bframe.h/5;
	/* NOTE: in a perfect world, the above would be equal. */
	int i = (y+uh/2)/uh; /* row */
	int j = (x+uw/2)/uw; /* col */
	if (0 <= i && i < 5 && 0 <= j && j < 4) {
		b->R.x = bframe.x + j*uw + ep;
		b->R.y = bframe.y + i*uh + ep;
		cout << i << "\t" << j << endl;
	}
	// 20 (e)s to represent an empty board:
	// string starting_state = "eeeeeeeeeeeeeeeeeeeee";

	// cout << "x: " << b->R.x << "\ty: " << b->R.y << endl;
	// x index = (b->R.x - 850)/216 ----> will give a number between 0-3
	// y index = (b->R.y - 182)/216 ----> will give a number between 0-4



}



//prevent current cell from being equal to e on the left or above

// ignore for now
int main(int argc, char *argv[])
{
	
	/* TODO: add option to specify starting state from cmd line? */
	/* start SDL; create window and such: */
	if(!init()) {
		printf( "Failed to initialize from main().\n" );
		return 1;
	}
	atexit(close);
	bool quit = false; 		 /* set this to exit main loop. */
	vector<string> solution; //storing the solution here
	size_t j=0;				 //will use to step through solution
	SDL_Event e;
	/* main loop: */
	while(!quit) {
		/* handle events */
		while(SDL_PollEvent(&e) != 0) {
			/* meta-q in i3, for example: */
			if(e.type == SDL_MOUSEMOTION) {
				if (mousestate == 1 && dragged) {
					int dx = e.button.x - lastm.x;
					int dy = e.button.y - lastm.y;
					lastm.x = e.button.x;
					lastm.y = e.button.y;
					dragged->R.x += dx;
					dragged->R.y += dy;
				}
			} else if (e.type == SDL_MOUSEBUTTONDOWN) {
				if (e.button.button == SDL_BUTTON_RIGHT) {
					block* b = findBlock(e.button.x,e.button.y);
					if (b) b->rotate();
				} else {
					mousestate = 1;
					lastm.x = e.button.x;
					lastm.y = e.button.y;
					dragged = findBlock(e.button.x,e.button.y);
				}
				/* XXX happens if during a drag, someone presses yet
				 * another mouse button??  Probably we should ignore it. */
			} else if (e.type == SDL_MOUSEBUTTONUP) {
				if (e.button.button == SDL_BUTTON_LEFT) {
					mousestate = 0;
					lastm.x = e.button.x;
					lastm.y = e.button.y;
					if (dragged) {
						/* snap to grid if nearest location is empty. */
						snap(dragged);
					}
					dragged = NULL;
				}
			} else if (e.type == SDL_QUIT) {
				quit = true;
			} else if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {
					case SDLK_ESCAPE:
					case SDLK_q:
						quit = true;
						break;
					case SDLK_LEFT:
						/* TODO: show previous step of solution */
						if (solution.size() == 0) break;
						
						if (j < solution.size()-1){
							j++;
							moveBlocks(solution[j]);
						}
						break;

					case SDLK_RIGHT:
						/* TODO: show next step of solution */
						if (j > 0){
							j--;
							moveBlocks(solution[j]);
						}
						break;

					case SDLK_p:
						/* TODO: print the state to stdout
						 * (maybe for debugging purposes...) */
						scanBlocks();
						break;

					case SDLK_s:
						/* TODO: try to find a solution */
						moveBlocks();
						solution = bfs(grid);
						j=solution.size()-1;
						break;

					case SDLK_m:
						//final position of the solution
						if (solution.size() == 0) break;
						moveBlocks(solution[solution.size()-1]);
						j = 0;

						break;
					case SDLK_b:
						moveBlocks();			
						if (solution.size() != 0) j = solution.size()-1;
						break;

					default:
						break;
				}
			}
		}
		fcount++;
		render();
	}

	printf("total frames rendered: %i\n",fcount);
	return 0;
}
