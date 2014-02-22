NTSC-composite-encoder
======================

![Screenshot](https://raw.github.com/elpuri/NTSC-composite-encoder/master/demo.jpg)

What is it?
-------------
This is a proof of concept level NTSC composite video encoder written in VHDL. This is by no means perfect, but it's a start. My main motivation for writing this is that I've been lately getting back into FPGA hacking and specifically retro computing (designing my own crappy CPUs and GPUs, emulating consoles, etc). Looking at my stuff being displayed on a shiny flat screen just looks a bit meh. At the risk of sounding like a someone trying to explain why vinyls sound better than CDs, I'm just gonna say a composite signal displayed on a cheap CRT TV just looks right. 

Gotchas
-------
I tried generating composite color signal few years back without success. Turns out my design was working just fine except one tiny detail. Back then I spent a lot of time googling around looking for reference implementations and reading descriptions of NTSC. Regardless of all the reading I did, I just couldn't get it right. Hopefully the following random bits of information will make things easier for people following my footsteps. I also tried to document the VHDL code really well so that it would be easier to understand what's going on.

* During my previous attempt to get colors working I thought (due to nice diagrams like this http://m.eet.com/media/1050360/C0195-Figure2.gif) that I had to generate 9 beautiful cycles of color burst starting and ending at the blanking level. To do just that I reset the counter that generated addresses to the sine table at the beginning of the color burst. Nope. You're supposed to keep the counter running and output whatever comes out of the sine table during the color burst period, so that the phase doesn't change (too much) between bursts. Resetting the phase at the beginning of every burst of course messeses things up. Back then I had no clue what a phase locked loop was and how the TV circuitry might use one. Now all of it makes sense.
* I used http://www.kolumbus.fi/pami1/video/pal_ntsc.html to get the timings and information about the vertical synchronization pulses. A great source of information for both PAL and NTSC. http://www.retroleum.co.uk/electronics-articles/pal-tv-timing-and-voltages/ is good too, but for PAL only.
* The key to getting that nice non-interlaced "mode" used by all the lovely HW from late 70s to mid 90s is to always send the same field synchronization pulses (they differ for the two fields). I'm not 100% sure what those equalization pulses are for and if most TVs actually do fine without them. Looking at how for example the TIA used in Atari 2600 works, I don't think it generates them. Maybe they play a part only when doing interlaced video. *
* The absolute voltage levels are not so important and things are relative. Looks like modern TVs are pretty good at adapting to almost any crap you throw at them. For example you can boost the saturation by using a slightly lower than nominal amplitude for the color burst.
* Not low pass filtering the luma component will generate color noise. If you have really sharp edges in your luma signal, you're inadvertently generating harmonics in the higher frequencies which get interpreted as color information by the TV. The same applies for the color information. The phenomenon is explained here http://en.wikipedia.org/wiki/Dot_crawl. Of course if you want yours look gritty like the cheap hardware did back in the day, then forget all this.


*) I tried leaving out the equalization pulses and things still work, but not completely without side-effects. At least my test TVs OSD doesn't like it and wobbles up and down one non-interlaced line.
