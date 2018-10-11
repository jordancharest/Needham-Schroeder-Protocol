# Needham-Schroeder Protocol

## Cloning
This repo relies on two submodules that I wrote, UDP-Server and DES, so you must be sure to clone the submodules as well. There are two options:
```bash
git clone --recurse-submodules -j8 <this-repository>
```
or
```bash
git clone <this-repository>
cd path/to/repo
git submodule update --init --recursive
```

## Building
To build you will need a c++14 compiler and CMake, both of which are built-in to many Linux distributions. I used GNU 5.4.0. To build:
```bash
cd path/to/repo
mkdir -p build
cd build
cmake ..
make
```
This will build three executables: KDC (Key Distribution Center), alice, and bob in the `build` directory.

## Running
To complete the entire Diffie-Hellman key exchange and then spin up the secure messaging channel using Needham-Schroeder Protocol, you will need to run all three executables. The order is important, since the KDC expects messages to arrive in a particular order. The program isn't optimized for user experience, since that's not the point of the exercise and I have to draw the line somewhere. This is the procedure:

Run the KDC. From the repository root:
```bash
./build/kdc alice.txt bob.txt
```
Contained in the text file command line arguments are Alice and Bob's public information (P and G) for Computational Diffie-Hellman Key Exchange. THE KDC will read the text files and generate public keys for Alice and Bob. Then it will wait for a connection request from Alice. To send the connection request from Alice, in another terminal, from repository root, run:
```bash
./build/alice alice.txt
```
The Alice program will read her public info from `alice.txt`, generate a value (more on this later) and immediately send it to the server. The server will then send Alice its own generated value, and they will both compute the (identical) private key. A key between Alice and the server has now been securely exchanged. Since the goal is to communicate with Bob, the server will then prompt Alice to send over the private key that she wishes to use in her communication with Bob. This traffic is of course encrypted using the private key that the server and Alice had previously exchanged. Type a 3-digit hex value in the terminal and press enter to send a response:
```
Received encrypted message: 濏ҏ]񿻷]*NI%*񿻷컀e췱컰*%a
*g
Decrypting...
Hello Alice, please input the 10-bit key you wish to use in your communication with Bob (3-digit hex):
123
```

Now that the server has the private key Alice wishes to use. It will wait to receive a connection request from Bob. At this point, run the Bob program:
```bash
./build/bob bob.txt
```

This will follow the same Computational Diffie-Hellman Key Exchange Protocol that Alice completed previously. After generating the private key, the server will prompt Bob to send his key over the (now encrypted) communication channel. Type a 3-digit hex value in the terminal and press enter to send a response: 

```bash
Received encrypted message: :I
IQc
GA

I
AIggNAK
K
N

                                                                     
IN

I
N
7_
L
Decrypting...
Hello Bob, Alice wishes to communicate. Please input the 10-bit key you wish to use (3-digit hex):
432

```

After receiving the private key from Bob, the server will generate two copies of a session key for Alice and Bob, one encrypted with Alice's private key, and one encrypted with Bob's private key, and send them both to Alice. Alice decrypts her copy of the session key and forwards Bob's copy to him. At this point, Alice and Bob both have the session key, and a secure chat session is started. Type whatever you want in either Alice or Bob's terminal and see the other receive the encrypted message and decrypt it!

## Computational Diffie-Hellman   
