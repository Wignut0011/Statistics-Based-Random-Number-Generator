#ifndef SOURCE_PICKER_H
#include "Libraries.h"

//Filter-like thread
//Will wait till new third foundation seed is generated
//then pick generator for Generator Thread.
struct Picker{
    //Wrapper function for exception handling
    static void PickerWork(){
        try{PickerWorkHelper();}
        catch(std::runtime_error &err){std::cerr << "\n\nFATAL ERROR IN GENERATOR THREAD: " << err.what() << "\n\nEXITING...\n\n" <<  std::endl;}
        catch(std::logic_error &err){std::cerr << "\n\nFATAL ERROR IN GENERATOR THREAD: For some reason, " << err.what() << "\n\nEXITING...\n\n" <<  std::endl;}
        catch(...){std::cerr << "UNKNOWN ERROR OCCURRED IN PICKER THREAD.\n\n";}
    }

    //Picker Thread
    static void PickerWorkHelper(){
        while(true){
            std::unique_lock<std::mutex> pickLck(firstLock);

            ///WAIT CONDITION - Until the seed thread populates a new foundation seed
            ///                 for the picker and seed thread is not done
            if (!seedFinished && foundationSeeds[firstSeed].empty())
                firstSeedNew.wait(pickLck, [](){return (!foundationSeeds[firstSeed].empty() || seedFinished || numFinished);});

            /// END CONDITION - Number was fully generated or there is no more to generate here
            if (numFinished || (seedFinished && foundationSeeds[firstSeed].empty())){
                pickFinished = true;
                //Unlock mutex
                pickLck.unlock();
                return;
            }

            long long pickerSeed = foundationSeeds[firstSeed].front();//Grab foundation seed
            foundationSeeds[firstSeed].pop(); //This seed is done for
            pickLck.unlock();//Unlock mutex lock

            //Generate picking number from seed
            NumberGenerators RandomGenerators;
            long long prePick = RandomGenerators(DEFAULT_GEN, pickerSeed);

            //Truncate number to generator range
            std::unique_lock<std::mutex> indGenLck(indLock);//Lock generator index mutex
            indPicker.push(fmod((prePick*1e-3), GENERATOR_TOTAL)); //Save index
            indGenLck.unlock();//Unlock generator index mutex
            indPickNew.notify_all(); //Notify Generator that new index was picked

            //Create insert index when the final num is populated a bit

                std::unique_lock<std::mutex> insGenLck(insLock); //Lock insert index mutex
                if (finalNum.empty())
                    insPickNew.wait(insGenLck, []() {return !finalNum.empty();}); //Wait until a num was inserted

                insPicker.push(prePick % finalNum.size());//Truncate result to end size of current number
                insGenLck.unlock();//Unlock insert index mutex
                insPickNew.notify_one();
        }///LOOP CONDITION - Go back to idle and wait until new foundation seed is created
    }
};
#define SOURCE_PICKER_H
#endif //SOURCE_PICKER_H
