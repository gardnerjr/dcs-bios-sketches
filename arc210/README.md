# ARC-210 psuedo-radio, aka SLK-210

## what is it?
The A10 in DCS does not have a functional ARC-210 radio, so in order to use that space and not have to build all 5 separate radio panels (UHF, VHF-AM, VHF-FM, TACAN, and ILS) i built something that *looks* like and ARC-210 but acts differently.
<a data-flickr-embed="true" href="https://www.flickr.com/photos/gardnerjr/51899756587/in/datetaken-public/" title="Untitled"><img src="https://live.staticflickr.com/65535/51899756587_224dd9cfb8_c.jpg" width="800" height="600" alt="the front"></a>

## what does it do?
All the rotaries are functional but do not do what the etched text says:
* the big upper right knob switches between radios, 
* lower right knob switches modes (off, transmit/receive, etc), 
* lower right switches between things like preset/manual modes, 
* lower center knob cycles presets or volume of that radio, pressing the rotary toggles what that knob does.
* the other 5 center rotaries cycle through frequencies.  

i believe the bottom 3 rotaries also have their pushbutton wired up, though i only have the middle one wired up to anything.

All of the things that *look* like buttons are exactly that, they just *look* like buttons.  they're etched so you see the outlines and text, but are just silver marker to look like buttons.

All of the rotaries cycle through whatever they do so it doesn't matter what position they are in at startup/etc.

On the screen, There's a big box in the upper left that shows the current radio and any state/mode info.
If the radio is off, the text will be gray.  If the radio is ON, the text will be green.  For the TACAN radio, if it is in one of the A/A modes, the text will be yellow.

Along the left edge shows which radio the controls apply to, with the same color scheme.  you can see in the pic that all fo the radios are on, except ILS.

The bottom edge shows preset info if that radio has it, and then what frequency that radio is set to.  Depending on the size/ranges, only some of the frequency knobs do anything.

That leaves some space in above the frequencies (which in the screenshot reads `vhf fm vol 0`) is where i draw some debug info, so i can see that the radio actually is responding to me doing things for testing.

## see it in action
It makes a lot more sense once you [see it in action (YouTube video)](https://www.youtube.com/watch?v=Aqy1hANtX4Y).


## what did i use?
* dcs-bios, the original fork, not the flightpanels fork. I haven't *needed* anything that's only in the flightpanels fork yet, so i haven't changed anything about my setup, i don't need to break something inadvertantly :D

* 9 rotary encoders [(something like this)](https://www.amazon.com/gp/product/B07D3D64X7)

* Seeeduino Mega (ATmega2560) [(like this)](https://www.seeedstudio.com/Seeeduino-Mega-ATmega2560.html)
  This mega is different than many!  I had this one for a different thing, because it has a switch that switches the board from using 3.3 or 5v, but the real interesting here is that it only has some of the pins already mounted into headers on one side, where the LCD shield would attach and cover up all the other pins.  This let me mount headers for the rest of the pins on the *back side*.
   <a data-flickr-embed="true" href="https://www.flickr.com/photos/gardnerjr/51899756707/in/datetaken-public/" title="Untitled"><img src="https://live.staticflickr.com/65535/51899756707_ea0550103d_c.jpg" width="800" height="600" alt="the mess at the back"></a>

* 3.5" TFT LCD shield [(like this one)](https://www.amazon.com/gp/product/B07NWH47PV) 
  The shield connects to standard pins on the "top" side of the mega, hiding much of the board.  This shield has all kinds of extra stuff like an SD card slot and touch that i don't use at all.
  (this specific shield i've had a so-so experience with. i've had one where the soldering job was horrible on on side and it came apart, and another had some kind of failure after a couple days where the screen went ghosted and i had to get another one)

* green 5v LED strip lights, cut and soldered and glued where they need to be/fit to backlight the panel.

* lots of wire and solder

* a 3d printer (Ender 3 v2, with lots of mods), used to print all the layers that things are mounted to, and the box it is inside of, and all the knobs.

  <a data-flickr-embed="true" href="https://www.flickr.com/photos/gardnerjr/51899756752/in/datetaken-public/" title="Untitled"><img src="https://live.staticflickr.com/65535/51899756752_329d90bb28_c.jpg" width="600" height="800" alt="the layers"></a>

  The rough layout is very similar to that of the panels done by [The Warthog Project](https://thewarthogproject.com/the-panel-design), though he laser cuts his and they come out very professional.  I 3d printed mine, the top 3 layers in clear filament (should have used white maybe?), painted them black, and then used a super cheap laser etcher (in several passes since it isn't big enough to do the whole panel!) to etch out the text, lines and outlines. there's 2 more layers inside, one that the led strip is glued to, and one that the mega is attached to

## how does it work?
* none of the encoders/buttons are mapped directly to dcs-bios buttons/etc like you'd normally do.
* instead, each loop checks to see if any of the encoders have changed, and if so, respond.
* some of that functionality is purely n the radio sketch, cycling internal state like which radio the rest of the controls will activate and which content to draw
* for reading data *from* dcs, there are various `DcsBios::StringBuffer` or `IntegerBuffer` that watch the state change, and then the content is copied into various arrays to draw later as necesssary.
* for writing back *to* dcs, the various methods call back using `sendDcsBiosMessage`, like `sendDcsBiosMessage("UHF_MODE", increase ? "INC" : "DEC")` to cycle through the UFH radio modes like a single click or right click with the mouse would do.


## wishlist
* i'm 100% sure there's a better way to wire up all the rotary encoders using a matrix or breakout board or something to not need all of the inputs (each encoder is 5 connections if you want the pushbutton, 3 if not, x 9 encoders!)
    * (but i only had 1 quarter of EE in college 20 years ago and didn't have the energy to learn that part again!)
* there's a fair amount of flickering depending on what's going on, would be nice to not have that
    * i try to draw over or redraw as little as possible, doing a full screen redraw almost never
    * that makes it better, but also the way dcs-bios sends data, sometimes it causes unecessary redraws
* i'd LOVE to actually design and build a pcb that has all of the encoders, pushbuttons and LED's mounted on it, so the buttons could be functional too.  i started looking into that given all the cool stuff you can do now with custom boards pretty cheap, but haven't found enough time to learn all that!
* i *believe* this could be adapted to work for other aircraft, right now it only handles the A10 variants. There is no problem in computer science that can't be solved by adding another layer of indirection, so in this case, it would be adding another layer to map which radios are available, which commands to send via send message, and possibly a LOT more Buffer objects that watch for specific messages for each different aircraft?