# CPPCrack

g++ build commands for each task:

task1: g++ -o luhn.exe luhn.cpp
To run ./luhn.exe

task2: g++ -o bcherrorcorrector.exe bcherrorcorrector.cpp
g++ -o bchgen.exe bchgen.cpp
To run ./bcherrorcorrector.exe or ./bchgen.exe respectively 

task3seta: g++ -o bfseta.exe bruteforceseta.cpp -I C:/openssl/include -L C:/openssl/lib -Wdeprecated-declarations -lssl -lcrypto 
To run ./bfseta.exe

task3setb: g++ -o bfsetb.exe bruteforcesetb.cpp -I C:/openssl/include -L C:/openssl/lib -Wdeprecated-declarations -lssl -lcrypto 
To run ./bfsetb.exe

task4: g++ -o rsa.exe rsa.cpp -I C:/openssl/include -L C:/openssl/lib -Wdeprecated-declarations -lssl -lcrypto 
To run ./rsa.exe
Use the format: D:\filename to store the keys/encrypted data. If just a directory name is entered the program does not handle the creation of a filename so it will print an error and continue runnning.

Note: to run each task you must cd into the respective task directory using 
> cd task1
> cd task 2, etc

Additionally, for task 3 onwards replace C:/openssl/include with your own OpenSSL library installation folder if it is different. 

How to install OpenSSL on Windows 10 per https://stackoverflow.com/questions/76331564/how-to-install-openssl-from-source-on-windows-10-11

Pre Requisites:

Step 1: Install Perl - Install the Strawberry version, much easier to install and it installs everything and also adds them automatically to the Windows PATH variables

Step 2: Install NASM, and add it to the Windows system (or your user's) PATH variables. I ended up adding it only to my user's variables PATH: C:\Users\<username>\AppData\Local\bin\NASM

Step 3: Install Visual Studio (I have Visual Studio Community 2022), and install the Desktop development with c++

Step 4: Download and install the Build Tools for Visual Studio (I assume in the future this link will change so look for the Build Tools installation link for your Visual Studio version): https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022 

Step 5: After installing the build tools, launch the Visual Studio installer. In the installer, you will now see the Build Tools. Click on "Modify" under the Visual Studio Build Tools: 
And then install the needed packages for the OpenSSL installation, it's what's going to install nmake:

Build and Install Steps:

Step 6: Clone the openssl repository to some folder on your PC (I cloned it in C:/ so I ended up having C:/openssl/), and fix the line endings by running the following commands:
> git clone git://git.openssl.org/openssl.git
> cd openssl
> git config core.autocrlf false
> git config core.eol lf
> git checkout .

Step 7: In Windows Search, search for "Developer Command Prompt for VS 2022" (Or any of your versions), and run it as administrator.

Step 8: You need to set the right environment for the version of OpenSSL you want to install, otherwise build will fail. In my case, I wanted to install OpenSSL for 64-bit systems, copy-paste the following (including the quotes, and change the path according to your Visual Studio installation path):
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
Once the environment is set the console should read at the bottom: 
[vcvarsall.bat] Environmenet initialised for: 'x64'

Step 9:  From the same Developer Command Prompt, cd into the folder you cloned the openssl source code, in my case it was C:/openssl, and then follow the steps from the OpenSSL guide:
> perl Configure VC-WIN64A
> nmake
> nmake test
> nmake install
Note: that these steps take time, it took me around 20-30 minutes to finish all these 4 commands

Step 10: That's it! It's installed! You can find the OpenSSL executable (openssl.exe) at C:\openssl\apps. (And add it to Windows system or user's PATH variables if you want)
In my case when I run openssl version I see OpenSSL 1.1.1q  5 Jul 2022
