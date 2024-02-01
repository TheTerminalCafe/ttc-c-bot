# TTC-C-Bot:
A small in-house discord bot for the TTC discord server. The library and the bot are both open source and free to use meaning you can use the bot in your server if you wish or the library in to develop your own bot if you want that as well.

## How to use it:
Library just run `make` and `sudo make install` the make file is almost 100% POSIX compiliant so should work on any make system. As for the bot just run `make` followed by `./ttc-bot` to get it up and running. See Running below

## Bug reporting:
We always welcome input and bug reports should you find anything:

## Technologies Used:
The bot currently has 5 dependencies 3 of which are in house for logging, websockets and HTTP handling. The other two out of house libraries being Openssl and json-c.

- ttc-http: A HTTP library written by us for the bot but can also be used independant of the bot.
- ttc-log: a small logging library that again was developed for the bot can be used on it's own.
- ttc-ws: a small websocket library that can be used on it's own.
- Openssl: allows for connect to an SSL socket rather than plain text.
- Json-c: easier Json handling in C.


## Running:
I know above I made it seem easy to run the bot but you do need to make your own config.ini file with your Discord app id and Bot token. once that is done you should be able to run the bot. See sample below.
```
TOKEN=YOURSUPERSECRETTOKEN
APP_ID=YOURAPPID
```
