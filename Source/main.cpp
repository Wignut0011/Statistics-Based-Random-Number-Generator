#include "Libraries.h"
#include "Seed.h"
#include "Picker.h"
#include "Generator.h"
#include <iostream>
#include <fstream>
#include <vector>

//Names for the threads inside the thread pool
enum ThreadIndexes{SeedThread = 0, PickerThread = 1, GeneratorThread = 2};
//Limit for printing
enum {
    PrintLimit = 32,
    FPS = (1000 / 16), //(1000 / fps)
    n = 100 //Change to increase file output
};
bool animationStop = false;

/*Little waiting animation " ─ \ | / "  */
void busyAnimation() {
    const char spin[] = {'|', '/', '-', '\\'}; // Simplified animation frames
    int frameIndex = 0;

    while (!animationStop) {
        std::cout << "\r" << "Loading Please Wait " << spin[frameIndex] << std::flush; //Print
        frameIndex = (frameIndex + 1) % 4; //Next index
        std::this_thread::sleep_for(std::chrono::milliseconds(FPS)); // Animation sleep timer
    }

    std::cout << " "; // Clear the last frame
}

void OneNum(size_t digitSize){
    std::vector<std::thread> threadPool;

    remainingSize = digitSize;

    try {    //Start Seed Thread
        threadPool.emplace_back(std::thread(Seeds::SeedWork));

        //Start Picker Thread
        threadPool.emplace_back(std::thread(Picker::PickerWork));

        //Start Generator Thread
        threadPool.emplace_back(std::thread(Generator::GeneratorWork));

        //Join and end program
        for (auto &i : threadPool) {
            i.join();
        }
    } catch (std::runtime_error &err) {
        std::cerr << "\n\nFATAL ERROR: " << err.what() << "\n\nEXITING...\n\n" << std::endl;
    }
    catch (std::logic_error &err) {
        std::cerr << "\n\nFATAL ERROR: For some reason, " << err.what() << "\n\nEXITING...\n\n" << std::endl;
    }
}

std::queue<std::string> MatLab(size_t n, size_t digitSize){
    std::string numArray;
    std::queue<std::string> fileAm;

    for (int i = 0; i < n; ++i) {
        while (finalNum.empty() || finalNum.size() != numSize){ //Make sure to restart if an error happened
            OneNum(digitSize);

            if(!finalNum.empty() && finalNum.size() == numSize) {
                numArray += (finalNum + "#");
                //Reset all statics
                resetStatics();
                break;
            }

            //Reset all statics
            resetStatics();
        }
//        //Fragment into multiple files
//        if (!(i%n) && i > 0){
//            numArray = numArray.substr(0,numArray.size()-1); //Get rid of delimiter end
//            fileAm.push(numArray);
//            numArray = "";
//        }
    }
    numArray = numArray.substr(0,numArray.size()-1); //Get rid of delimiter end

    if (fileAm.empty() || fileAm.front() != numArray)
        fileAm.push(numArray);

    return fileAm;
}

void FileSave(std::string output, std::string file){
    do {
        std::fstream save(file, std::fstream::out | std::fstream::trunc);
        if (save.is_open()) { //File stream open, save number
            save << output;
            save.close();
        } else { //Could not open the file stream, input a file name again
            std::cout << "\nERROR: Could not save with the file name " << file << std::endl;
            file = "";
        }
    } while (file.empty());
}

int main() {
    //Ask user if they want a specific size
    std::cout << "Welcome, how many digits do you want your randomly generated number to contain?\n> " << std::flush;
    size_t digitSize = 0;
    while(!digitSize){
        //Get input
        std::string input;
        std::cin >> input;

//        input = "30";//Debug

        //Check if valid
        bool isValid = std::all_of(input.begin(), input.end(),[](char i){return isdigit(i);});
        if (isValid && std::stoi(input) >= MIN_DIGIT)
            digitSize = std::stoi(input);

        //Not valid
        else {
            std::cout << "That was not a valid entry, please type a size ranging from " << MIN_DIGIT << " to " << INT_MAX << ".\n> " << std::flush;
        }
    }

    std::cout << "\nWould you like to export batch of 100 numbers to a text file? (y/n): " << std::flush;
    char response;
    std::cin >> response;
    while(response != 'y' && response != 'Y' && response != 'n' && response != 'N'){
        std::cout << "\n\nThat was not a valid entry, please type 'y' or 'n': " << std::flush;
    }

    std::cout << "Please Wait  " << std::flush;
    numSize = digitSize;
//    remainingSize = digitSize;

    //Small animation thread, send generator thread reference
    std::thread pleaseWait(busyAnimation);


    if (response == 'n' || response == 'N'){

        OneNum(digitSize);

        //Stop animation
        animationStop = true;
        pleaseWait.join();

        std::cout << " FINISHED!\n\nYour generated number is: " << std::flush;
        //Digit too long to display
        if (digitSize > PrintLimit) {
            std::cout << "Your number was too big to be displayed here!" << std::endl;
        }
        //Number can be printed on the console
        else
            std::cout << finalNum << std::endl;

        //Save number to file
        std::cout << "\nFor convenience, your number will be placed into a .txt file.\n"
                     "Note: If you use an existing file name, IT WILL BE OVERWRITTEN!\n"
                     "\nPlease enter a file name: " << std::flush;

        std::string file;
        std::cin >> file;
        file += ".txt";

        FileSave(finalNum, file);
    }

    else {
        size_t lim = n/100;
        if (n <= 100)
            lim = 1;
        std::queue<std::string> numArray = MatLab(n, digitSize);

        //Stop animation
        animationStop = true;
        pleaseWait.join();

        for (int i = 0; i < lim; ++i) {
            FileSave(numArray.front(), std::string("Output_"+ std::to_string(i)+".txt"));
            numArray.pop();
        }
//
//        FileSave(numArray.front(), std::string("Output.txt"));
//        numArray.pop();
        std::cout << " FINISHED!\n\nFile saved to Output.txt\n*Remember to use '#' as the delimiter" << std::flush;
    }
    std::cout << "\n\nPress enter to exit..." << std::flush;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    std::cin.get();
    return 0;
}
