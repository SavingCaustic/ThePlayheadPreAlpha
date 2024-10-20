Files related to real-time playback serivce.

We'll see later about the PreRenderPlayer, if we will need it or not..

PreRenderPlayer:
* Multithreaded
* Not connected to midi or mic.
* Buffersizes of >1024 samples.
* Off-loads PlayerEngine so rack-audio pre-rendered audio are simply copied on mixdown. 