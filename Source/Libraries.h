#ifndef SOURCE_LIBRARIES_H

//ALL LIBRARIES WILL BE DEFINED HERE

//Containers
#include <string>
#include <queue>

//Error Handling
#include <cassert> //Assertions
#include <iostream>


//Multi-Threading
#include <thread>
#include <mutex>
#include <condition_variable>


//Calculations
#include <functional>
#include <algorithm> //Useful algorithms
#include <chrono> //Time
#include <cmath> //Mathematical operations


//Noise Generator
#include "Libraries/FastNoiseLite/FastNoiseLite.h" //Noise Generator -Credit: Auburn via: https://github.com/Auburn/FastNoiseLite

//*--------------------------------------------------------------------------------------------------------------------------------------------------*
//*--------------------------------------------------------------------------------------------------------------------------------------------------*

///Global Variables Defined Here

//Type alias, more randomness is found the smaller the time-scale. If you want a higher time scale for some reason,
//then you can change to a different chrono type here.
using TIME_TYPE =  std::chrono::nanoseconds;
enum {
    INT_MAX_PLACES = 10
};

enum {
    MIN_DIGIT = 1,
    EVEN = 2,
    ODD = 1,
    CHUNK_BAIS = 2, //Higher number means more chunks, MORE CHUNKS ARE BETTER
    MIN_CHUNK = 3 //Smallest number to loop for a chunk amount, just finish at this point
};

//Names for the cores seeds
enum FoundationNames{
    firstSeed = 0, //Creates chunk sizes
    secondSeed = 1, //Responsible for main generator shuffle and insert position
    thirdSeed = 2, //Main generator seed
    initSeed = 3, //Responsible for creating initial noise map
    SEED_AMOUNT = 3
};

//Non-integral constants
const double TRUNC_DOUBLE = 0.1;


const double MOD_F_PERM = 1e3;
const double MOD_PERM = 1e-3;
const double D_PERM = 1e7;
//Thread related variables
static bool numFinished, seedFinished, pickFinished = false; //Is the generator done?

static std::condition_variable firstSeedNew, thirdSeedNew, chunkPickNew, indPickNew, insPickNew;
static std::mutex firstLock, thirdLock, chunkLock, indLock, insLock;
static std::queue<size_t> indPicker, insPicker; //Index picker for Picker/Generator threads, -1 = refresh needed
static size_t remainingSize = 0;
static size_t numSize = 0;
static std::queue<size_t> chunkSize; //How much each generated chunk of numbers will be used
//Seed related variables
//static bool seedRefresh[3] = {false, false, false};

static std::queue<long long> foundationSeeds[4]; //Core seeds.
//The final random number string
static std::string finalNum;

//*--------------------------------------------------------------------------------------------------------------------------------------------------*


//*--------------------------------------------------------------------------------------------------------------------------------------------------*

///RANDOM NUMBER GENERATORS
enum {
    GENERATOR_TOTAL = 10, //Total amount of included generators UPDATE IF MORE ARE ADDED
    GEN_SELECTION_TOL = 100, //Digit tolerance for selecting a random number generator
    DEFAULT_GEN = 1 //Default random generator
};

///PUT YOUR GENERATORS HERE

#include <cstdlib> //rand()
#include <random> //Cpp Engines
/*
 *       #|                Generator Name                |      Library         |                       Reference Link
 *      --|----------------------------------------------|----------------------|-------------------------------------------------------------------
 *       0|  rand()                                      | The old c library    | https://www.cplusplus.com/reference/cstdlib/rand/?kw=rand
 *       1|  Default Random Engine                       | C++ <random> library | https://www.cplusplus.com/reference/random/default_random_engine/
 *       2|  Linear Congruent Engine Prime Multiplier    | C++ <random> library | https://www.cplusplus.com/reference/random/minstd_rand/
 *       3|  Linear Congruent Engine (7^5) Multiplier    | C++ <random> library | https://www.cplusplus.com/reference/random/minstd_rand0/
 *       4|  Mersenne Twister Engine (32 bit)            | C++ <random> library | https://www.cplusplus.com/reference/random/mt19937/
 *       5|  Mersenne Twister Engine (64 bit)            | C++ <random> library | https://www.cplusplus.com/reference/random/mt19937_64/
 *       6|  Subtract With Carry Engine (24 bit)         | C++ <random> library | https://www.cplusplus.com/reference/random/ranlux24/
 *       7|  Subtract With Carry Base Engine (24 bit)    | C++ <random> library | https://www.cplusplus.com/reference/random/ranlux24_base/
 *       8|  Subtract With Carry Engine (48 bit)         | C++ <random> library | https://www.cplusplus.com/reference/random/ranlux48/
 *       9|  Subtract With Carry Base Engine (48 bit)    | C++ <random> library | https://www.cplusplus.com/reference/random/ranlux48_base/
 */

//When creating multiple
void resetStatics(){
    //Queues
    //Foundation seeds
    for(auto &i : foundationSeeds) {
        while (!i.empty())
            i.pop();
    }

    //Chunks
    while (!chunkSize.empty())
        chunkSize.pop();

    //Generator Index
    while (!indPicker.empty())
        indPicker.pop();

    //Insert Index
    while (!insPicker.empty())
        insPicker.pop();

    //Flags
    seedFinished = false;
    pickFinished = false;
    numFinished = false;

    //Mutexes
    firstLock.unlock();
    thirdLock.unlock();
    chunkLock.unlock();
    indLock.unlock();
    insLock.unlock();

    //Generated number
    remainingSize = 0;
    finalNum = "";
}

class NumberGenerators{
    std::function<long long(long long)> C_RandomGen = [](long long seed){
        srand(seed);
        return rand();
    };
    std::function<long long(long long)> CppDefault_RandomGen = [](long long seed){
        std::default_random_engine randy(seed);
        return randy();
    };
    std::function<long long(long long)> LinCong_RandomGen = [](long long seed){
        std::minstd_rand randy(seed);
        return randy();
    };
    std::function<long long(long long)> LinCong0_RandomGen = [](long long seed){
        std::minstd_rand0 randy(seed);
        return randy();
    };
    std::function<long long(long long)> MersTwist_RandomGen = [](long long seed){
        std::mt19937 randy(seed);
        return randy();
    };
    std::function<long long(long long)> MersTwist64_RandomGen = [](long long seed){
        std::mt19937_64 randy(seed);
        return randy();
    };
    std::function<long long(long long)> SubCarry24_RandomGen = [](long long seed){
        std::ranlux24 randy(seed);
        return randy();
    };
    std::function<long long(long long)> SubCarry24B_RandomGen = [](long long seed){
        std::ranlux24_base randy(seed);
        return randy();
    };
    std::function<long long(long long)> SubCarry48_RandomGen = [](long long seed){
        std::ranlux48 randy(seed);
        return randy();
    };
    std::function<long long(long long)> SubCarry48B_RandomGen = [](long long seed){
        std::ranlux48_base randy(seed);
        return randy();
    };

    const std::function<long long(long long)> RandomGenerators[GENERATOR_TOTAL] ={C_RandomGen, CppDefault_RandomGen, LinCong_RandomGen, LinCong0_RandomGen, MersTwist_RandomGen, MersTwist64_RandomGen, SubCarry24_RandomGen, SubCarry24B_RandomGen, SubCarry48_RandomGen, SubCarry48B_RandomGen};

public:

    long long operator()(size_t index, long long seed){return RandomGenerators[index](seed);}
};

/// Work Functions. IT DON'T MATTER IF THEY ARE NOT EFFICIENT, IT'S JUST WORK.
/// ^^IF WANT TO CHANGE THE WORK POOL, UPDATE THE ENUMS ON TOP^^

/*Index:
* 1: Sin division
* 2: Cos division
* 3: Tan division
* 4: Root division
* 5: Log division
 *
 * Selection:
 *      Divide tolerance into equal pieces.
 *          IE: Tol is 100, 5 different work functions, each function is
 *              seperated into sections 0-20, 20-40, 40-60, 60-80, 80-99
 *
 *      Subsection of currTime is extracted and will act as our selection.
 *          IE: currTime was 15315656.4156315, 41 was extracted, 41 falls into function 3
*/

enum Totals{ //Amount of modules in a particular pool
    WORK_TOTAL=5,
    NOISE_PROFILE_TOTAL = 6
};

enum Tolerances { //Tolerances - MUST BE AT LEAST DECIMAL HIGHER THEN TOTAL IN A POOL
    S_TOL = 100,
    N_FTOL = 100
};

//enum DecSelection{TRUNC_DOUBLE = 0.1, MOD_PERM = 1e3}; //MOD_PERM must shift decimals to match tolerances.

enum Defaults{ //Defaults if selection loops don't execute a function.
    FOCTAVE_DEFAULT = 1,
    W_DEFAULT = 0,
    W_MAX_LOOP = 100
};

class WorkFunctions{
const std::function<void(double)> SinDiv = [](double wSeed) { //Sin Division
    //auto test = (size_t) ((int) fmod((wSeed * MOD_F_PERM), 100) + 1 > 1 ? (fmod((wSeed * MOD_F_PERM), W_MAX_LOOP)) : (2)); //Debug the loop condition
    for (size_t i = 1; i < (size_t) ((int) fmod((wSeed * MOD_F_PERM), 100) + 1 > 1 ? (fmod((wSeed * MOD_F_PERM), W_MAX_LOOP)) : (2)); ++i) {        (wSeed * (double)i) / (sin(wSeed) + (double)i);
        if (i > W_MAX_LOOP)
            break; //In case loops too much
    }
};
const std::function<void(double)> CosDiv = [](double wSeed) { //Cos Division
    //auto test = (size_t) ((int) fmod((wSeed * MOD_F_PERM), 100) + 1 > 1 ? (fmod((wSeed * MOD_F_PERM), W_MAX_LOOP)) : (2)); //Debug the loop condition
    for (size_t i = 1; i < (size_t) ((int) fmod((wSeed * MOD_F_PERM), 100) + 1 > 1 ? (fmod((wSeed * MOD_F_PERM), W_MAX_LOOP)) : (2)); ++i) {        (wSeed * (double)i) / (cos(wSeed) + (double)i);
        if (i > W_MAX_LOOP)
            break; //In case loops too much
    }
};
const std::function<void(double)> TanDiv = [](double wSeed) { //Tan Division
    //auto test = (size_t) ((int) fmod((wSeed * MOD_F_PERM), 100) + 1 > 1 ? (fmod((wSeed * MOD_F_PERM), W_MAX_LOOP)) : (2)); //Debug the loop condition
    for (size_t i = 1; i < (size_t) ((int) fmod((wSeed * MOD_F_PERM), 100) + 1 > 1 ? (fmod((wSeed * MOD_F_PERM), W_MAX_LOOP)) : (2)); ++i) {        (wSeed * (double)i) / (tan(wSeed) + (double)i);
        if (i > W_MAX_LOOP)
            break; //In case loops too much
    }
};
const std::function<void(double)> RtDiv = [](double wSeed) { //Square Root Division
    //auto test = (size_t) ((int) fmod((wSeed * MOD_F_PERM), 100) + 1 > 1 ? (fmod((wSeed * MOD_F_PERM), W_MAX_LOOP)) : (2)); //Debug the loop condition
    for (size_t i = 1; i < (size_t) ((int) fmod((wSeed * MOD_F_PERM), 100) + 1 > 1 ? (fmod((wSeed * MOD_F_PERM), W_MAX_LOOP)) : (2)); ++i) {        (wSeed * (double)i) / (sqrt(wSeed) + (double)i);
        if (i > W_MAX_LOOP)
            break; //In case loops too much
    }
};
const std::function<void(double)> LogDiv = [](double wSeed) { //Log Division
    //auto test = (size_t) ((int) fmod((wSeed * MOD_F_PERM), 100) + 1 > 1 ? (fmod((wSeed * MOD_F_PERM), W_MAX_LOOP)) : (2)); //Debug the loop condition
    for (size_t i = 1; i < (size_t) ((int) fmod((wSeed * MOD_F_PERM), 100) + 1 > 1 ? (fmod((wSeed * MOD_F_PERM), W_MAX_LOOP)) : (2)); ++i) {
        (wSeed * (double) i) / (log(wSeed) + (double) i);
        if (i > W_MAX_LOOP)
            break; //In case loops too much
    }
};
    const std::vector<std::function<void(double)>> UselessFunctions{SinDiv, CosDiv, TanDiv, RtDiv, LogDiv};

public:
    std::function<void(double)> operator[](size_t index){return UselessFunctions[index];}
};

#define SOURCE_LIBRARIES_H
#endif //SOURCE_LIBRARIES_H
