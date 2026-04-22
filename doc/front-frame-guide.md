# Front frame

Here you will learn how to assemble the front frame. Note that some parts can be a little different if you are doing a DIY protopanda instead of an assembled one with the PCB.

1) [Tools](#tools)
1) [Panels](#Panels)
1) [HUB75 Cables](#hub75-cables)
1) [Power cable](#power-cable)
1) [Other cables](#other-cables)
1) [Internal screen](#internal-screen)
1) [Boop sensor](#boop-sensor)
1) [Cooler](#cooler)
1) [Second panel](#second-panel)
1) [Fur support](#fur-support)
1) [Connector cover](#connector-cover)
1) [Fresnel lens](#fresnel-lens)
1) [USB Cable](#usb-cable)
1) [Materials](#materials)


## Materials

The first and most important material is the printed front frame for the P2.5 matrix.

Then you will need the electronics:

* 2x [P2.5 HUB75 led panels](https://pt.aliexpress.com/item/1005005793147869.html)
* 2x HUB75 cables (usually come with the panel)
* 1x double cable for the HUB75 matrixes (usually come with the panel)
* 1x [4010 5v 40mmx10cm fan](https://pt.aliexpress.com/item/1005009148488355.html)
* 1x USB cable
* 1x [0.96 inch oled screen](https://pt.aliexpress.com/item/1005006262908701.html)
* Some cables
* 4x M2 6mm screws
* 4x M2 nut
* 2x M3 16mm~18mm
* 4~6x M3 8mm screws
* 2x [M3 nut self tapping](https://pt.aliexpress.com/item/1005008207473770.html)


![img](./front-frame-materials.png)

### Optionals

* 2x [IDC 8x2 terminal](https://pt.aliexpress.com/item/1005007995752212.html)
* 2x M3 nuts
* 2x M3 8MM screws
* 2x M3 4MM screws
* [Fresnel lens](https://pt.aliexpress.com/item/1005009696060069.html)
* [KF2510](https://pt.aliexpress.com/item/1005011814149800.html?) Get the 4 pin one.

### Tools

* Screwdriver
* Tools for crimping and crimping terminals (depends on how you gonna do it)

# Asembly guide

## Panels

First we need to disassemble the panels to remove the original support. Use a thin screwdriver
There are a total of 6 screws, they're really tiny, so be careful and save them.

![img](./front-frame-screw-1.png)

Once removed take off the support.

![img](./front-frame-screw-2.png)

Now its time to put one of the panels in the LEFT side of the frame.

![img](./front-frame-fixing-panel-1.png)

There is a correct orientation of the panel. Note on the panel there are white arrows. Align them facing forward like the red arrows. Also align the panel uusing that hole pointed by the yellow arrow.

Then follow those steps:
1) Align the panel
2) Place the screw in the hole
3) Push the screw an align the panel
4) With a small screwdriver or a screwdriver bit, tighten the screw. **DONT TIGHT TOO MUCH, just enough to be in place**
5) Do it for those 5 points highlighted in red.

![img](./front-frame-fixing-panel-2.png)

## HUB75 Cables

Now its time to put the first cable. This cable will transfer the signal from the first panel to the second one.

![img](./front-frame-cable-1.png)

But as you can see, thats a bit too much. You can leave it as is, or you can trim the cable down. Thats whats I'll teach you right now. But please pay attention on what you doing, if you wire them backwards you will certainly destroy one or both panels.

If you want to leave it as is, skip to [Power cable](#power-cable)  

As you can see, the cable has about 19~20cm. But we need at least 11CM. 

![img](./front-frame-cable-2.png)

Then for that, you will need to cut the cable at about 13.5cm~14.5cm

![img](./front-frame-cable-3.png)

Then grab the IDC terminal and put it in the correct orientation. **DOUBLE OR TRIPLE CHECK THE ORIENTATION BASED IN THE ORIGINAL CABLE**

![img](./front-frame-cable-4.png)
![img](./front-frame-cable-5.png)

If you need extra help, try googling on how to crimp idc connectors.

## Power cable

For powering the HUB75, you can use the original cable. But its bulky and extra long. So you can trim it down. 

![img](./front-frame-cable-6.png)

Then cut right when it joins with the other cable

![img](./front-frame-cable-7.png)

After that remove the extra terminals, we only need one black and one red on each of those two.

![img](./front-frame-cable-8.png)

Then strip about 1cm of each wire, twist them together, put a shrink wrap on each of them and crimp a fork terminal like this:
![img](./front-frame-cable-10.png)

Thats how they should look like:

![img](./front-frame-cable-9.png)

They go in the protopanda at the screw terminal.

![img](./front-frame-cable-11.png)

If you're doing DIY route, they go at the schematic where is written **HUB75 POWER**
In case istn clear, red means positive and black negative.

![img](./front-frame-cable-12.png)

## Other cables

Now its a good idea do plug the other cables.

![img](./front-frame-cable-13.png)

If you doing the DIY route, you can choose any kind of connector you want, or just wire directly to the display and boop sensor. I used PHT connector but that was my prefference for the way I did. Do your way!

![img](./front-frame-stuff-1.png)

Specifically for proto panda controller, there is a spot dedicated to it. But the DIY route might take up more space. That part will depend heavily on what and how you built it. The screw holes are M2.

![img](./front-frame-stuff-3.png)

With a long screwdriver you can each all 3 screws.

![img](./front-frame-stuff-4.png)

Then it should look like this.

![img](./front-frame-stuff-5.png)


## Internal screen

You can totally use the screen with the pin header. But personally, i like to use those [KF2510](https://pt.aliexpress.com/item/1005011814149800.html) connectors. 

I soldered the 90 degree connector and cut off the pins that came out the other side.

![img](./front-frame-stuff-6.png)

Then grab the screws and the nuts to attach it to the frame:

![img](./front-frame-stuff-7.png)

Put the screws trough the holes

![img](./front-frame-stuff-8.png)

And screw them tto the nuts at the other side

![img](./front-frame-stuff-9.png)

## Boop sensor

There will be a guide to make the boop sensor in the future. I'll do it i swear! For now it just the capacitive sensor with some wire wrapped around it.

![img](./front-frame-boop-1.png)

Attach the boop sensor to the front o the panel

![img](./front-frame-boop-2.png)


## Cooler

For the cooler, we're using 2x M3 16mm and the nuts. First connect the fan to the socket. If doing the DIY route, use the same pins as the HUB75 power.

![img](./front-frame-fan-1.png)

Then place the fan on the bottom of the front frame, with the wires facing forward.

![img](./front-frame-fan-2.png)
![img](./front-frame-fan-3.png)

Use the nuts at the bottom part and tight the screws.

![img](./front-frame-fan-4.png)

## Second panel

Now we're not fiddling with any more wiring, lets connect the last cables on the second panel. PAY ATTENTION ON THE DIRECTION OF THE ARROWS IN THE PANEL.

Therer shold be 4 screws to fix it in place. They are circled in yellow.

![img](front-frame-panel-fix-1.png)

No need the 5th one. The process to align the panel is the same as the previous one.

## Fur support

In case you a place to sew or attach the fur, there is a support.

![img](./front-frame-spp-1.png)

Use the M3 8mm screws and the nuts to fix it in place. It should snap in place but needs the screw to stay there.

![img](./front-frame-spp-2.png)
![img](./front-frame-spp-3.png)

## Connector cover

Since we dont wanna damage the HUB75 connectors with our cheeks, lets add a cover for them.
Use 2x M3 8MM screws to fix it on the front frame 

![img](./front-frame-cover-1.png)

It should look like this:

![img](./front-frame-cover-2.png)

## Fresnel lens

The fresnel lens is completely opitional but a nice to have. Without it might be hard to focus your vision at the internal screen. Witth it is way way easier. The effects are mostly in person.

![img](./front-frame-fresne-1.png)

For assembling it, you will need to cut the fresnel in a 64x34cm rectangle. Glue it in the 3d printed slot using super glue (JUST A TINY DROP)

![img](./front-frame-fresne-2.png)

Screw a 8MM M3 screw to attach both parts together

,![img](./front-frame-fresne-3.png)

Then use the two M3 5MM screws and put them in the hole of the other part

![img](./front-frame-fresne-4.png)

Now align it in the front frame. It goes at the internal right top part of it.

![img](./front-frame-fresne-5.png)

Then screw it in place and you will have an fresnel lens that can be moved up out of the way when the visor is not there, or simply unscrewd in case you dont like it!

![img](./front-frame-fresne-6.png)

## USB Cable

Now for the USB cable. In the material list i asked for a 90 degree USB-C. There is not much space for a straight USB cable there, and that might force the cable or the connector. Even the side facing cable can have alot of strain on it. 

![img](./front-frame-usb-1.png)

Thats why we will palce it in a slot.

![img](./front-frame-usb-2.png)

Make sure the cable is going inside that slot and doing a little curve inside. If needed push it a bit inside,

![img](./front-frame-usb-3.png)

# IS COMPLETE!

![img](./front-frame-completed.png)

**There you go! You completed it!!**