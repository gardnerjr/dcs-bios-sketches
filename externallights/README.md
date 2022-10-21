# External Lights

## what is it?
A super simple test of driving some LEDs for the external lights of an A10.
<a data-flickr-embed="true" href="https://www.flickr.com/photos/gardnerjr/51416001809/in/datetaken-public/" title="Untitled"><img src="https://live.staticflickr.com/65535/51416001809_967ebd5e4e_c.jpg" width="800" height="600" alt="Untitled"></a>

## what does it do?
Lights up 4 LED's
* a white strobe on each side (surprisingly bright for what they are)
* a red on the left
* a green on the right

In the pic above you can see the red light on the edge of my desktop panel, and the green on the other side lighting up the wall.
The white LED's blink, so you can't see them there.

## why, though?
Do you know how often i forget to turn them off when i FENCE IN? way too often.

## see it in action
It makes even more sense once you [see it in action (Flickr video)](https://www.flickr.com/photos/gardnerjr/51415511468/in/album-72157715786645217/)

## what did i use?
* dcs-bios, the original fork, not the flightpanels fork. I haven't *needed* anything that's only in the flightpanels fork yet, so i haven't changed anything about my setup, i don't need to break something inadvertantly :D

* 4 LEDs that came with one of the Arduino UNO kits i used

* An arduino nano to test it, after i got it working i merged the 4 lines into my larger code that runs on a mega and drives a ton of other things.

* wire and resisters?

## how does it work?
* honestly, i think there's more bytes here alerady than the code, so just (look at that)[externalights.ino]

## wishlist
* in game, the red and green lights "pulse" on and off (though i think the switch in the cockpit says "flash"?).  
  I don't know if DCS bios doesn't handle it properly, or what, so they're either on or off, it isn't on/off/pulse like you'll see in game.