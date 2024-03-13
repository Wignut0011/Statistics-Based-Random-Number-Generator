#ifndef SOURCE_GENERATOR_H
#include "Libraries.h"

//Consumer-like thread
//Will wait till new first foundation seed and picker seed are generated
//then will used picked generator and insert result.
struct Generator{
    //Wrapper function for exception handling
    static void GeneratorWork(){
        try{GeneratorWorkHelper();}
        catch(std::runtime_error &err){std::cerr << "\n\nFATAL ERROR IN GENERATOR THREAD: " << err.what() << "\n\nEXITING...\n\n" <<  std::endl;}
        catch(std::logic_error &err){std::cerr << "\n\nFATAL ERROR IN GENERATOR THREAD: For some reason, " << err.what() << "\n\nEXITING...\n\n" <<  std::endl;}
        catch(...){std::cerr << "UNKNOWN ERROR OCCURRED IN GENERATOR THREAD.\n\n";}
    }

    //Generator Thread
    static void GeneratorWorkHelper(){
        while(true){
            std::unique_lock<std::mutex> pickLck(thirdLock);

            ///WAIT CONDITION 1 - Until the seed thread populates foundation seed
            if (foundationSeeds[thirdSeed].empty() && !seedFinished)
                thirdSeedNew.wait(pickLck, [](){return (!foundationSeeds[thirdSeed].empty() || seedFinished || pickFinished);});

            //Throw if there is no remaining size
            else if (foundationSeeds[thirdSeed].empty() && seedFinished)
                throw std::runtime_error("Generator detected a remaining size of 0 with no foundation seeds. Race "
                                         "condition.");

            //Throw if new seed was not created
            if (foundationSeeds[thirdSeed].empty())
                throw std::runtime_error("Generator thread woke up to consume it's foundation seed, but the queue"
                                         " was not refreshed");

            long long gSeed = foundationSeeds[thirdSeed].front();//Grab foundation seed
            foundationSeeds[thirdSeed].pop(); //This seed is done for
            pickLck.unlock();//Unlock foundation mutex lock


            std::unique_lock<std::mutex> indPickLck(indLock);//Lock Generator Index Mutex

            ///WAIT CONDITION 2 - Generator index is not populated and Picker thread is not done
            if (!pickFinished && indPicker.empty())
                indPickNew.wait(indPickLck, [](){return (!indPicker.empty() || pickFinished);});

            else if(pickFinished && indPicker.empty())
                throw std::logic_error("Generator thread is at a state to consume a generator index, but\n"
                                       "the picker thread is done and the queue is empty");

            //Throw if the generator index pick is out of bounds
            if (indPicker.front() >= GENERATOR_TOTAL)
                throw std::runtime_error("Generator thread woke up to consume indPicker, but indPicker was " +
                std::to_string(indPicker.front()) + " but there are only " +
                std::to_string(GENERATOR_TOTAL) + " total generators.");

            //Consume generator index for local use
            size_t currGenInd = indPicker.front();
            indPicker.pop(); //Consumed index, pop it
            indPickLck.unlock(); //Unlock mutex

            std::unique_lock<std::mutex> consumeChunk(chunkLock);

            ///WAIT CONDITION 3 - Chunk is not populated and Seed thread is not done
            if (!seedFinished && chunkSize.empty())
                chunkPickNew.wait(consumeChunk, [](){return (!chunkSize.empty() || pickFinished);}); //This is important to get right, so wait here

            //Throw if chunk queue is empty and see thread is done for
            else if (seedFinished && chunkSize.empty())
                throw std::logic_error("Generator thread is at a state to consumer chunkSize, but\n"
                                       "the seed thread is done and the queue is empty.");

            //Throw if chunkSize was not updated
            if (chunkSize.empty()) {
                throw std::runtime_error("Generator thread woke up to consume chunkSize, despite chunkSize being empty");
            }

            //Consume chunk for local use
            size_t currChunk = chunkSize.front();
            chunkSize.pop(); //Consumed chunk, pop it
            consumeChunk.unlock(); //Unlock mutex

            //Generate block
            NumberGenerators RandomGenerators;
             std::string block = std::to_string(abs(RandomGenerators(currGenInd, gSeed)));//Grab index selection and generate
            while (block.size() < currChunk) //It's too small
                block += std::to_string(abs(RandomGenerators(currGenInd, stoi(block))));//Grab index selection and generate

            //Make sure that block was generated
            if (block.empty())
                throw std::logic_error("block had exited the generation loop empty.");

            //Get substring of generated number in the middle, sliced to the chunk size
            if ((block.size()-currChunk) % EVEN) //Odd split
                block = block.substr((block.size()-currChunk)/2+1, currChunk);
            else
                block = block.substr((block.size()-currChunk)/2, currChunk);

            // Insert to position if Picker Thread chose one, or just emplace back
            //*We are not worried about order here, so let race conditions happen
            if (insPicker.empty())
                finalNum.append(block);
            else {
                finalNum.insert(insPicker.front(), block);
                insPicker.pop();
            }

            //Notify picker thread to start creating insert indexes
            if (insPicker.empty())
                insPickNew.notify_one();

            /// END CONDITION - Number was fully generated
            if (finalNum.size() == numSize){
                numFinished = true;
                //Notify Seed and Picker threads in-case
                firstSeedNew.notify_all();
                return;
            }
        }///LOOP CONDITION - Go back to idle and wait until new foundation seed and picked index are created
    }
};
#define SOURCE_GENERATOR_H
#endif //SOURCE_GENERATOR_H