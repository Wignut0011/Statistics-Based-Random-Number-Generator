#ifndef SOURCE_SEED_H
#include "Libraries.h"




class Seeds {
    inline static NumberGenerators RandomGenerators; //Random Number Generators
    inline static FastNoiseLite noiseMap; //Noise Map
    inline static double currTime_double; //Storage for the double version of currTime

    //Time
    std::chrono::system_clock currTime;

public:
    static void SeedWork() {
        ///INIT SEED -PRE-CONDITION
        //Get system time
        auto currTime = std::chrono::system_clock::now();
        currTime_double = currTime.time_since_epoch().count();
        assert(currTime_double);

        //Perform calculation
        do{ //Through testing, there is a small possibility that the clock does not progress when timing
            auto t_start = std::chrono::steady_clock::now();
            DoUselessWork(currTime_double);
            auto t_end = std::chrono::steady_clock::now();
            foundationSeeds[initSeed].push(std::chrono::duration_cast<TIME_TYPE>(t_end - t_start).count()); //Got initial seed
        } while (foundationSeeds[initSeed].empty());

        ///MAIN THREAD BEGIN
        while (true){
            /// EXIT CONDITION  -No more seeds or chunks to produce
            if (!remainingSize) {
                seedFinished = true;
                firstSeedNew.notify_all(); //Notify Generator Thread
                thirdSeedNew.notify_all(); //Notify Picker Thread
                return;
            }
            GenerateSeeds(); //Refresh the seeds.
            firstSeedNew.notify_all();
            thirdSeedNew.notify_all();

        }    ///LOOP CONDITION
    }

private:
    //Re-populates foundation seeds
    //Mid-Condition: Initial Seed has been created.
    static void GenerateSeeds() {
        assert(!foundationSeeds[initSeed].empty()); //initSeed needs to be initialized

        if (foundationSeeds[firstSeed].empty()) //Three foundation seeds have not been generated yet, start with the initseed
            foundationSeeds[thirdSeed].push(foundationSeeds[initSeed].front());

        //Generate the Three Foundation seeds and send previous foundation seed as a profile shuffle
        for (size_t i = 0; i < SEED_AMOUNT; ++i) {
            long long timeInd;

            if (i == firstSeed) {
                std::unique_lock<std::mutex> genLck(thirdLock); //Lock for Picker thread
                timeInd = foundationSeeds[thirdSeed].front();

                //Make sure temp init seed in third seed is removed
                if(foundationSeeds[thirdSeed].front() == foundationSeeds[initSeed].front())
                    foundationSeeds[thirdSeed].pop();
            }
            else if(i == secondSeed){
                std::unique_lock<std::mutex> genLck(firstLock); //Lock for Generator thread
                timeInd = foundationSeeds[firstSeed].front();
            }
            else
                timeInd = foundationSeeds[secondSeed].front();

            auto t_start = std::chrono::steady_clock::now();
            DoUselessWork(timeInd % WORK_TOTAL-1);
            auto t_end = std::chrono::steady_clock::now();

            long long timeSeed = std::chrono::duration_cast<TIME_TYPE>(t_end - t_start).count();

//            foundationSeeds[i].push((i==0) ? (NoiseSeed(foundationSeeds[thirdSeed].front())) : (NoiseSeed(foundationSeeds[i-1].front())));
            foundationSeeds[i].push(NoiseSeed(timeSeed));

//            //Make sure temp init seed in third seed is removed
//            if (i == firstSeed && foundationSeeds[thirdSeed].front() == foundationSeeds[initSeed].front())
//                foundationSeeds[thirdSeed].pop();

            if(i == secondSeed) //Make sure to update the next chunk size
                ChunkSizeUpdate(foundationSeeds[i].front());

            //If foundation seed was not generated, throw.
            if ( i != secondSeed && foundationSeeds[i].empty())
                throw std::runtime_error("Foundation seed " + std::to_string(FoundationNames(i)) + " could not "
                                             "be generated.\nFoundation Seed: " + std::to_string(foundationSeeds[i].front()));

        }
    }

    //Sets the noise profile and returns foundation seed
    static long long NoiseSeed(const long long &set_map){
        if(set_map != foundationSeeds[initSeed].front()){ //Returning for a noise map, change the profile.
            //Noise Type -6 enum types
            int nSelect = round(fmod((currTime_double * MOD_PERM), N_FTOL)); //Shifted numbers left by 3 and modulo by work tolerance
            int nCondition = N_FTOL / NOISE_PROFILE_TOTAL; //Set up index conditions
            bool noiseWasPicked = false;
            for (size_t i = 0; i < NOISE_PROFILE_TOTAL-1; ++i) {
                if (nCondition*i <= nSelect && nSelect <= nCondition*(i+1)) {
                    noiseMap.SetNoiseType((FastNoiseLite::NoiseType) i); //Select noise profile
                    noiseWasPicked = true;
                }
            }
            if (!noiseWasPicked)
                noiseMap.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2); //Noise profile wasn't selected, pick default

                noiseMap.SetFrequency(fmod(set_map,.1) * MOD_PERM); //Frequency
            noiseMap.SetFractalOctaves((((int)(set_map * MOD_PERM) % 10) % 5 > 0)? (((int)(set_map*1e3) % 10) % 5):(FOCTAVE_DEFAULT)); //Fractal Octave -int  *limit to 5 for looped work, make sure it's not 0
            noiseMap.SetFractalLacunarity((float)set_map);//Fractal Lacunarity -float
            noiseMap.SetFractalGain(fmod((float)(set_map * MOD_PERM), N_FTOL) * MOD_PERM);//Fractal Gain -float
        }

        noiseMap.SetSeed((int)set_map); // *Narrow conversion is intended for more randomization
        float noiseSum = 0;
        long long lim = foundationSeeds[initSeed].front()%100+1;
        if(lim < 2){
            lim = (int)(foundationSeeds[initSeed].front()*MOD_PERM) % 100+1;
        } //Sometimes chrono steady_clock at nanosecond size produces a xx00 number

        //Noise map is a matrix. Iterate through a lim x lim loop
        for (size_t x = 0; x <= lim; ++x) {
            for (size_t y = 0 ; y <= lim; ++y) {
                noiseSum += noiseMap.GetNoise((float)x, (float)y);
            }
        }
        return abs((noiseSum*D_PERM)/fmod(foundationSeeds[initSeed].front(), (N_FTOL+1)));
    }

    static void ChunkSizeUpdate(long long sSeed){
        //Random number[0, REMAINING] / BIAS(we don't want the chunks too big)
        size_t selection = ODD+1; //Just to initialize
        std::unique_lock<std::mutex> updateChunk(chunkLock); //Lock mutex
        do {
            //Break if we are at the minimum chunk threshold
            if (remainingSize <= MIN_CHUNK){
                selection = remainingSize;
                break;
            }
            selection = ((size_t)(RandomGenerators(DEFAULT_GEN, sSeed) * MOD_PERM) % remainingSize) / CHUNK_BAIS;
            sSeed++;
        }while((selection % ODD) || selection > MIN_DIGIT || selection > INT_MAX_PLACES || selection > remainingSize || selection < 1); //SHRINK CRITERIA && MIN

        //Update chunk size
        chunkSize.push(selection); //Update
        remainingSize -= selection;
        updateChunk.unlock(); //Unlock mutex
        chunkPickNew.notify_one(); //Notify thread
        foundationSeeds[secondSeed].pop();
    }

    //Does work to be timed. After all, this is the closest I can get to a truly random seed
    static void DoUselessWork(double wSeed){
        int wSelect = GenSelect(wSeed);
        int wCondition = (int)S_TOL / (int)WORK_TOTAL; //Set up index conditions
        bool workWasDone = false;
        WorkFunctions wFunc; //Work functions

        for (size_t i = 0; i < WORK_TOTAL; ++i) { //Pick matching selection through a conditional matching loop
            if(wCondition*i <= wSelect && wSelect < wCondition*(i+1)) {
//                UselessFunctions[i](wSeed);
                wFunc[i](wSeed);//Do the work no one asked for nor wanted nor will be thankful for but is needed ( ._.)
                workWasDone = true;
                break;
            }
        }
        if (!workWasDone) //Work function did not execute, use default
            wFunc[W_DEFAULT](currTime_double);
    }

    //Small helper function that returns a number for the selection range.
    static int GenSelect(double sSeed){
        return (int)round(fmod(sSeed, TRUNC_DOUBLE) * MOD_F_PERM); //Truncate numbers to
    }
};


#define SOURCE_SEED_H

#endif //SOURCE_SEED_H
