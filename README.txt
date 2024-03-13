  __  __  ____  _____  _    _ _               _____                                    
 |  \/  |/ __ \|  __ \| |  | | |        /\   |  __ \                                   
 | \  / | |  | | |  | | |  | | |       /  \  | |__) |                                  
 | |\/| | |  | | |  | | |  | | |      / /\ \ |  _  /                                   
 | |  | | |__| | |__| | |__| | |____ / ____ \| | \ \                                   
 |_|__|_|\____/|_____/ \____/|______/_/    \_|_|_ \_\       _   _ _____   ____  __  __ 
 |  __ \ / ____| |  | |  __ \ / __ \       |  __ \    /\   | \ | |  __ \ / __ \|  \/  |
 | |__) | (___ | |  | | |  | | |  | |______| |__) |  /  \  |  \| | |  | | |  | | \  / |
 |  ___/ \___ \| |  | | |  | | |  | |______|  _  /  / /\ \ | . ` | |  | | |  | | |\/| |
 | |     ____) | |__| | |__| | |__| |      | | \ \ / ____ \| |\  | |__| | |__| | |  | |
 |_|  _ |_____/_\____/|_____/_\____/____   |_|  \_/_/    \_|_| \_|_____/ \____/|_|  |_|
 | \ | | |  | |  \/  |  _ \|  ____|  __ \                                              
 |  \| | |  | | \  / | |_) | |__  | |__) |                                             
 | . ` | |  | | |\/| |  _ <|  __| |  _  /                                              
 | |\  | |__| | |  | | |_) | |____| | \ \                                              
 |_|_\_|\____/|_|  |_|____/|______|_|  \_\_______ ____  _____                          
  / ____|  ____| \ | |  ____|  __ \    /\|__   __/ __ \|  __ \                         
 | |  __| |__  |  \| | |__  | |__) |  /  \  | | | |  | | |__) |                        
 | | |_ |  __| | . ` |  __| |  _  /  / /\ \ | | | |  | |  _  /                         
 | |__| | |____| |\  | |____| | \ \ / ____ \| | | |__| | | \ \                         
  \_____|______|_| \_|______|_|  \_/_/    \_|_|  \____/|_|  \_\                        
                                                                                       
___________________________________________________________________________________________________________________

Executable Compatibility: Windows 10

How to run:
	Simply run NumGen.exe and follow the instructions on the command window.
	If this fails, you can build the executable. Instructions below.
	
	
Code Portability: GCC

How to build:
	Using your IDE of choice, you need to be using the latest version of the MinGW-64 compiler which can be found here: http://mingw-w64.org/doku.php/download
	IMPORTANT: If you are running Windows, be sure to select MingW-W64-builds and not Win-Builds. Win-Builds is outdated and abandonded
	
	Launch the installer	
	Once you run the settings part of the installer, be sure to select the appropriate selections:
		Latest version (currently 8.1.0)
		win32 for Threads (you may pick posix if you are not on Windows. However, I have not tested if posix will work)
		x86_64 if running a 64 bit operating system or i686 if running on a 32-bit operating system (I did not test i686 viability)
		Leave Exception on 'seh'
		Build revision on 0 (there are no build revisions for 8.1.0 anyway)
		
	Click Next
	
	IMPORTANT: Keep in mind where it's being installed to. You will need to know for later
	
	Click Next			
	*downloading and installing will take a few minutes.
	
	If you get a message saying "Downloaded incorrectly", then just run the installer again. This seems to be a recent serverside bug

	
	Now, using your IDE of choice, make sure to change the compiler/toolchain to the newly installed compiler.
	*Since IDEs are different, you will need to follow a tutorial on how to add it. Here are some examples:
	
	*Clion
	https://www.jetbrains.com/help/clion/how-to-create-toolchain-in-clion.html
	
	*Visual Code Studio (It references MSYS2, but follow the instructions anyway)
	https://code.visualstudio.com/docs/cpp/config-mingw
	
	Finally, be sure to add -static to your compiler flags if you plan on runing the .exe outside of your IDE.
	
	*Clion
	Add "target_link_libraries(Source PRIVATE -static)" in CMakeLists.txt
	
	*Visual Studio
	Project Properties > Configuration Properties > C/C++ > Command Line > add "-static" in Additional Options