#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <queue>
#include <algorithm>
using std::swap;
using std::queue;
using std::string;
using std::cout;
using std::endl;
using std::vector;
using std::unordered_map;
#include <fstream>
using std::ofstream;
using std::fopen;
using std::fclose;
using std::fwrite;

#include <bits/stdc++.h>
using std::time;
using std::clock;
using std::clock_t;
using std::setprecision;

void printV(vector<vector<string> >& grid);
vector<vector<string> > string_to_vector(string& s);
string game_won(const vector<string>& V);
string vector_to_string(vector<vector<string> >& v);


/*1-5-> rectangles
  6-9-> squares 
  e-> empty space
  b-> big square */

const int WIDTH = 4;
const int HEIGHT = 5;

// Original position
// vector<vector <string>> grid  = {{"1", "b", "b", "2"},
//                                  {"1", "b", "b", "2"},
//                                  {"3", "5", "5", "4"},
//                                  {"3", "6", "7", "4"},
//                                  {"8", "e", "e", "9"}};

//100 steps
// vector<vector <string>> grid  = {{"1", "b", "b", "2"},
//                                  {"1", "b", "b", "2"},
//                                  {"3", "6", "5", "5"},
//                                  {"3", "9", "4", "7"},
//                                  {"e", "8", "4", "e"}};
// sample[b]!= sample[b+4]

// 18 steps
// vector<vector <string>> grid  = {{"e", "1", "4", "2"},
//                                  {"3", "1", "4", "2"},
//                                  {"3", "7", "b", "b"},
//                                  {"e", "6", "b", "b"},
//                                  {"5", "5", "8", "9"}};

//64 steps
vector<vector <string>> grid  = {{"4", "b", "b", "9"},
                                 {"4", "b", "b", "8"},
                                 {"1", "7", "e", "3"},
                                 {"1", "6", "2", "3"},
                                 {"5", "5", "2", "e"}};

/* above: i-4
   below: i+4
   left:  i-1
   right: i+1 
   left/right edge of the board: i%4 == 0
   lower edge of the board: i-4 >= 0
   upper edge of the board: i+4 <= 19
   */

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
    for (int i=0; i < vertex.length(); i++){
        // maximum number of neighbors for e is 4

        if (vertex[i] == 'e'){
            //make copy of vertext to modify and push to vector n
            string sample = vertex;
            
            // case 1: right of empty space
            if (vertex[i] == 'e' && (i)%4!=3 && vertex[i+1] != 'e'){    
                right(sample, i, i+1);
                n.push_back(sample);
            }
            // case 2: left of empty space 
            if (vertex[i] == 'e' && (i)%4!=0 && vertex[i-1] != 'e'){
                sample = vertex;
                left(sample, i, i-1);
                n.push_back(sample);
            }
            // case 3: above empty space
            if (vertex[i] == 'e' && i-4 >= 0 && vertex[i-4] != 'e'){
                sample = vertex;
                above(sample, i, i-4);
                n.push_back(sample);
            }
            // case 4: below empty space
            if (vertex[i] == 'e' && i+4 <= 19 && vertex[i+4] != 'e'){
                sample = vertex;
                below(sample, i, i+4);
                n.push_back(sample);
            }
        } 
    }
            return n;
}

vector<string> getPath(string final_config, unordered_map <string,string>& came_from){

    vector<string> path;
    path.push_back(final_config);

    string temp=final_config;
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

vector<string> bfs(vector<vector<string> >& grid){
    string sample = vector_to_string(grid);
    
    queue<string> frontier;
    frontier.push(sample);
    // map to store board configurations
    unordered_map <string,string> came_from = {{sample, "None"}};

    clock_t start, end;
    start = clock();
    double total=0;
    double time_taken;
    int j=0;
    while (frontier.empty()==false){
        string currentS = frontier.front();
        frontier.pop();

        //11 seconds total to get neighbors from all loops
        j++;

        vector<string> neighbors = getNeighbors(currentS); //getting possible moves to make from the current configuration
        for (size_t i=0; i<neighbors.size(); i++){ //iterating through the available moves we can make
        
            //checks if we found the winning configuration
            if (won(neighbors[i])==true){
                came_from.insert({neighbors[i], currentS});
                cout << "j: " << j << endl;
                // cout << "\ntime getting solution: " << total << setprecision(5) << endl;
                return getPath(neighbors[i], came_from);   //getting all moves for the solution
                // return;
            }
            /*if neighbors[i] is a configuration that we see for the first time, add it to the queue 
            and keep track of its parent */
            // start = clock();

            //16 seconds total from all loops
            else if (came_from.count(neighbors[i]) == 0){
                frontier.push(neighbors[i]);
                came_from.insert({neighbors[i], currentS});
            }
            // end = clock();
            // time_taken = double (end-start)/double(CLOCKS_PER_SEC);
            // total+=time_taken;
        }
        
    }
    // while loop terminates if there's no path to winning
    cout << "no solution found\n";
    vector<string> path;
    return path;
}

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

void printV(vector<vector<string> >& grid){
    for (size_t i = 0; i<grid.size(); i++){
        for (size_t j = 0; j < grid[i].size(); j++){
            cout << grid[i][j] << "\t"; 
        }
        cout << endl;
    }
    cout << endl;
}

void printF(vector<string>& V, ofstream& myfile){
    
    myfile.open ("example.txt");
    myfile << "right, left, up, and down\n\n";
    for (size_t i = V.size()-1; i >= 0; i--){
        myfile << "step " << i << endl;
        vector<vector<string> > temp = string_to_vector(V[i]);
        for (size_t j = 0; j < temp.size(); j++){
            for (size_t k = 0; k < temp[j].size(); k++){
                myfile << temp[j][k] << "\t";
            }
            myfile << endl;
        }
        myfile << endl;

    }
  
}


// to check if block has height=2: add 4 to string index
// to check if block has width=2: add 1 to string index to check if same value
// to check if block height & width = 2: add both 4 and 1 to check if same value 
// to check if block height & width = 1: neither adding 4 or 1 to index would give us same value

void move(){
    unordered_map<char, char> map;
    vector<vector<string> > S = grid;

    string state = "1bb21bb2355436748ee9";
    for (int i = 0; i < state.length(); i++ ){
        
        if (map.find(state[i]) == map.end())
        {
            
            if (state[i] == state[i+1] || (state[i] == state[i+4] && i+4 < 20)){
                map[state[i]] =state[i];   
            }
            else cout << state[i] << " ";
        }
    }
    cout << endl;
    map.clear();
    for (size_t i=0; i<state.length(); i++){
		if (map.find(state[i]) == map.end())
        {
            //big square
            if (state[i]!='e' && state[i] == (state[i+1] && state[i+4]) && (i+4 < 20)){
                map[state[i]]=state[i];
				cout << state[i] << " ";
            }
			//horizontal rectangle
            else if (state[i]!= 'e' && state[i] == state[i+1]){
                map[state[i]]=state[i];
				cout << state[i] << " ";
            }
			//vertical rectangle
			else if(state[i]!='e' && state[i] == state[i+4] && i+4 < 20){
				map[state[i]]=state[i];
				cout << state[i] << " ";
			}
		}
	}
}


int main(){

    // cout << 17/4 << endl;

    clock_t start, end;

    // start = clock();
    // end = clock();
    // double time_taken = double (end-start)/double(CLOCKS_PER_SEC);

    // cout << "\ntime: " << time_taken << setprecision(5) << endl;
    
    vector<string> solution = bfs(grid);

    unordered_map<int, int> m;
    m[3] = 0;
    m[5] = 10;

    // cout << m[5];

    // cout << m.count(5);
    // string s ="1";
    // cout << s.empty();
	// vector<string> s(1, "no solution found");
    // cout << s[0];

    // ofstream myfile;
	// string state =  vector_to_string(grid);
    // // vector<string> n = getNeighbors(state);
    // printF(solution, myfile);
    // myfile.close();

    // for (size_t i=0; i < n.size(); i++){
    //     cout << n[i] << endl;
    // }

    // move();
    return 0;
}
