# Dungeons & Deadlines

A mediocre dungeon crawling experience that allows players to explore the exciting world of text-based adventure!

## Compiling the Game
To compile the game, you need to compile the game and the server separately. Use `gcc main.c aes.c -o game -lcrypto` to compile the neccessary files for the game and produce the `game` binary.  To compile the server, `cd` into `server/` and run `gcc main.c -o server` to produce the `server` binary.

## Running the game
To run the game, you need to start the server and then run the game using `./game` and `./server` respectively. By default, the client tries to connect to localhost to find the server, so you need to modify the game source and input your desired IP address if it isn't local.

