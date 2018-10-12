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
./build/bob bob.txt [<time-to-live>]
```
The optional third parameter (time to live) is the time in milliseconds that Bob allows to pass between his current timestamp and the timestamp he receives from Alice (from the KDC) before he declares that a replay attack has occurred. By default, it is 100 milliseconds. Testing on my computer shows the typical difference is about 2 milliseconds. A simple way to check if the replay protection is working is to set this to a negative number.
This program will follow the same Computational Diffie-Hellman Key Exchange Protocol that Alice completed previously. After generating the private key, the server will prompt Bob to send his key over the (now encrypted) communication channel. Type a 3-digit hex value in the terminal and press enter to send a response: 

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
The first part of this program involves two secure key exchanges, one between the server and Alice, and one between the server and Bob. This is achieved via the computational Diffie-Hellman key exchange protocol. How does this work? Alice chooses a generator (G) and a large prime number (P). The generator is usually a generator of some algebraic group, such as the multiplicative group of a finite field. Generators that form a full cycle in a cyclic group are generally the best choice to make. I do not know how to easily verify whether or not this is the case, so I chose my generators rather arbitrarily. Each end user uses this public information and a random, private number (a) and computes:

![equation](https://latex.codecogs.com/gif.latex?x%20%3D%20G%5Ea%20%5Cmod%20P)

The two parties then send their own result to each other (on an insecure communication channel). They receive the others generated value (y) and then compute the private key with the equation:

![equation](https://latex.codecogs.com/gif.latex?K_p_r_i_v_a_t_e%20%3D%20y%5Ea%20%5Cmod%20P)

In my implementation, only the 10 least significant bits are used as the master key to the DES encryption.

## Needham Schroeder Protocol
The Needham Schroeder protocol is very simple. After a secure communication channel is established between the server and Alice and the server and Bob, the two users can then send the server the private keys they wish to use fto set up the communication with each other. The server accepts the two private keys and generates two copies of a session key, one encrypted with Alice's private key and one encrypted with Bob's. A timestamp is also encrypted with Bob's session key, so that when Bob receives the key, he can be sure that the key is fresh, thus preventing a replay attack. The server sends both copies to Alice, who decrypts her own and forwards Bob's to him. Once they both decrypt the session key, they can communicate with each other securely.

## Security
This is a toy implementation of some cryptographic algorithms and is not secure in the slightest. First, the computational Diffie-Hellman implementation only allows primes up to 64 bits for ease of computation (and I used significantly smaller primes than that. Second, the encryption cipher used is DES with a 10-bit key, which can be determined via brute-force in probably a few milliseconds (1024 combinations). Surprisingly, the secure messaging does provide reasonable protection against replay attacks. This is achieved by encrypting and sending a timestamp along with each message. If the receiver receives the message after the message has expired (100 milliseconds after the timestamp), then the message is discarded. Of course, this is super easy to do when I am running it only on my own machine, and my clocks are synced. In reality, that small amount of delay is much too small. If I am chatting with a friend overseas, then perfectly valid messages could expire before they even arrive. Not to mention the difficulty/impossibility of actually syncing clocks in a distributed system. 

That being said, it is generally advised that people should not implement their own cryptography, and I am certainly no exception. Please, do not use this for anything but fun.
