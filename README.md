# nanoWeb

Web graphical user interface in Python and C++.

## Architecture

NanoWeb is not a library but two. Part of your application logic runs
on the client's browser and the rest on the server side. You can
decide how much logic runs in each side, but there are some trade
offs:

- The more logic you add in the client, the more responsive and
  latency immune it will be. But also it will result in a bigger
  javascript module you have to send to the client, so the startup
  time could be slower.

- Adding logic in the client side also alleviates the server load.

- On the other side, it's easier to grow the logic in the server and
  store huge amounts of data that can be queried by the client
  incrementally.

Client part is written in C++ and can be extended in C or C++
naturally. Python is the language of choice for the server part, but
can be easily replaced by other technologies.

For testing purposes, the client or frontend part can also be compiled
for the desktop, having the exact appearance and behaviour than in the
web.

When compiling for the web, the C and C++ code is 'transcompiled' into
javascript by means of `emscripten`.

## Compilation

NanoWeb uses CMake for the client side. There is a convenient script
in the root directory to compile the client part for the web and the
desktop in release and debug modes: `build_all.sh`. It will generate
the build artifacts in the directories:
`build_<debug|release>_<web|desktop>`.

## Hello world!

For a simple test of usage, follow these instructions:

- Compile client side part as explained in previous section.

- Go to `.../example/hello_world` and execute:
  `$ python -m SimpleHTTPServer 9999`

- In a browser, just navigate to: `http://127.0.0.1:9999`.

- You should see the Hello world! test example in the browser.

- To check the desktop frontend, execute
  `.../build_release_desktop/nanoWeb` while maintaining the web server
  running.

## Work in progress

This is a very young project and still in progress. Any collaboration is welcomed. Thanks!