#include <array>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <string>
#include <iostream>
#include <string>
#include <iomanip>
#include <condition_variable>
#include <vector>
#include <unordered_map>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <cstring>
using namespace std;

// Defalut configuration
int number_of_drinking_sessions = 20;
int number_of_philosophers = 5;
vector<vector<int>> input_matrix = {
    {0,1,0,0,1},
    {1,0,1,0,0},
    {0,1,0,1,0},
    {0,0,1,0,1},
    {1,0,0,1,0},
};
// Global variables
mutex g_lockprint;
atomic_bool start(false);
vector<vector<int>> bottles;
vector<thread> threads;

/*-------------------------bottle class------------------------------*/

class bottle
{
public:
    
    int            id;         // Bottle ID
    atomic<bool>   dirty;      // Bottle state: dirty or not
    mutex          mtx;
    condition_variable cv;
    
    bottle(int const bottleId):
    id(bottleId), dirty(true)
    {}
    
    
    // Request for a Bottle
    // If dirty: get bottle, update state to clean
    // if clean: get blocked, wait for signal
    void request()
    {
        unique_lock<mutex> lck(mtx);
        while(!dirty) cv.wait(lck);
        dirty = false;
    }
    
    
    // Finish using bottle, notify waiting philosophers
    void done_using()
    {
        unique_lock<mutex> ulock(mtx);
        dirty = true;
        cv.notify_all();
    }
    
};


/*------------------------Philosopher function----------------------------*/

void philosopher(int id, vector<bottle*> & bottle_list){
    
    srand((unsigned)time(NULL));
    
    while (!start.load()); // spin
    int count = 0; // Number of finished eating sessions
    do
    {
        // Think: sleep for a while
        usleep(rand()%1000+1);
        
        for (auto& b: bottle_list) b->request();
        g_lockprint.lock();
        cout << "philosopher " << id << " drinking" << std::endl;
        g_lockprint.unlock();
        //Eat: sleep for a while
        usleep(rand()%1000+1);
        
        g_lockprint.lock();
        cout << "philosopher " << id << " thinking" << std::endl;
        g_lockprint.unlock();
        
        for (auto& b: bottle_list) b->done_using();
        count++;
        
    } while (count<number_of_drinking_sessions);
    
}

/*--------------------------Read Command line----------------------------*/

string read_Input(int argc, char* argv[]){
    // Read Input and check
    string filename;
    string error_message = "";
    int c;
    while ((c = getopt(argc, argv, "s:"))!=-1) {
        switch (c) {
            case 's':
                number_of_drinking_sessions = stoi(optarg);
                break;
            default:
                break;
        }
    }
    if(argc>3){
    if(optind==argc-1){
        if (strcmp(argv[optind], "-")==0) {
            input_matrix.clear();
            //read from standard input
            string line;
            int count_edge = 0;
            cout << "Type in your configuration: "<<endl;
            while(cin){
                getline(cin, line);
                if(line.length()==1){
                    number_of_philosophers = stoi(line);
                    int n = number_of_philosophers;
                    input_matrix.resize(n,vector<int>(n));
                }
                else{
                    istringstream iss(line);
                    int p1,p2;
                    if(!(iss >> p1 >> p2))break;
                    if(input_matrix[p1-1][p2-1]==1||input_matrix[p2-1][p1-1]==1){
                        error_message = "Duplicate edges in configuration file!";
                        return error_message;
                    }
                    input_matrix[p1-1][p2-1] = 1;
                    input_matrix[p2-1][p1-1] = 1;
                    count_edge++;
                }
            }
            int n = number_of_philosophers;
            // Check file
            if(count_edge<n-1||count_edge>n*(n-1)/2){
                error_message = "Invalid Configuration";
                return error_message;
            }
            
        }
        else{
            input_matrix.clear();
            filename = argv[optind];
            // Read from file
            ifstream infile(filename);
            int count_edge = 0;
            string line;
            while (getline(infile, line)) {
                if(line.length()==1){
                    number_of_philosophers = stoi(line);
                    int n = number_of_philosophers;
                    input_matrix.resize(n,vector<int>(n));
                }else{
                    istringstream iss(line);
                    int p1,p2;
                    if(!(iss >> p1 >> p2))break;
                    if(input_matrix[p1-1][p2-1]==1||input_matrix[p2-1][p1-1]==1){
                        error_message = "Duplicate edges in configuration file!";
                        return error_message;
                    }
                    input_matrix[p1-1][p2-1] = 1;
                    input_matrix[p2-1][p1-1] = 1;
                    count_edge++;
                }
            }
            int n = number_of_philosophers;
            // Check file
            if(count_edge<n-1||count_edge>n*(n-1)/2){
                error_message = "Invalid Configuration";
                return error_message;
            }
        }
    }
    else{
        error_message = "Invalid Input! Please try again!";
        return error_message;
    }
    }
    return error_message;
}

/*-----------------------------Main function-------------------------------*/

int main(int argc, char * argv[]) {
    
    // Read Input from command line arguments

    if (argc>1) {
        string error = read_Input(argc,argv);
        if(error.length()!=0){
            cout << error <<endl;
            return 0;
        }
    }
    // Initializing bottle matrix
    int n = number_of_philosophers;
    
    bottles.resize(n);
    int bottle_id = 1;
    for(int i=0;i<n;i++){
        bottles[i].resize(n);
        for(int j=i;j<n;j++){
            if(input_matrix[i][j]==1){
                bottles[i][j] = bottle_id;
                bottle_id++;
            }
        }
    }
    
    // Initializing philosopher bottle list
    unordered_map<int, vector<bottle*>> philosopher_bottles;
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            if(bottles[i][j]!=0){
                int bottle_id = bottles[i][j] - 1;
                bottle* b = new bottle(bottle_id);
                philosopher_bottles[i].push_back(b);
                philosopher_bottles[j].push_back(b);
            }
        }
    }
    
    // Initialize threads
    for(int i=1;i<=number_of_philosophers;i++) threads.push_back(thread(philosopher,i,ref(philosopher_bottles[i-1])));

    // Start eating
    start = true;
    
    // Wait for each thread to finish
    for(auto& t:threads) t.join();
    
}
