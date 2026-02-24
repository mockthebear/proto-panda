# Print guide and assembly guide

Printing protopanda is easier than the original MK16 model. It requires less support and has some features like to reduce the amount of material used.

**The basic requirement is a 3d printer with 25x25cm bed.** Generally speaking, the whole set can be printed entirely in PLA. But that is not reccomended for some parts due strenght issues.
Be aware that PLA under directly sunlight can bend and warp, also it can easily become maleable on temperatures over 50ºC. 
Some parts of the guide will reccoment using ASA or ABS, but they're toxic while printing. Weight your decisions and needs.

Bellow you can click on each of those images. Each session has a printing guide, reccomended settings for each part, tools and accessories necessary and assembly guide.

| Front frame                                     | Headphones                                          | Back head                                   |
|-------------------------------------------------|------------------------------------------------------|---------------------------------------------|
| ![Front frame](./guide-front-frame.png)         |  ![Headphones](./guide-front-headphones.png)        |  ![Back head](./guide-front-headbackpng.png)|
| Materials         |  [Materials](#materials-headphones)        |  Materials|
| Printing         |  [Printing](#headphones-printing-guide)        |  Printing|
| Assembly         |  [Assembly](#headphones-assembly-guide)        |  Assembly|

## All tools required

* 3D printer
* Orca slicer (or some slicer with similar features)
* Soldering iron with a heatset tip
* 4x M4 10mm screws (temporary)
* Screwdriver
* Set of straight cutting pliers
* C clamp (optional)

### All consumables required
* Two component glue
* 10x M3 heat insert (5~7mm in height)
* 18x M3 heat insert 3mm height
* ASA filaament or PLA filament
* 2x 10mmX2mm neodimium magnet


# Headpones

## Headphones Materials

### Tools
* 3D printer
* Orca slicer (or some slicer with similar features)
* Soldering iron with a heatset tip
* M4 10mm screws (temporary)
* Screwdriver
* Set of straight cutting pliers
* C clamp (optional)

### Headphones Consumables 
* Two component glue
* 10x M3 heat insert (4~6mm in height)
* ASA filaament or PLA filament
* 2x 10mmX2mm neodimium magnet
* 6x M3 heat insert 3mm height

## Headphones Printing guide

### Part 1
The headphones can totally be printed in PLA. It will come out nice and it don't have thin parts that can easily break or crack. **Ideally use ASA.** You can use PETG, but petg is notoriously hard to sand and just a little moisture can make your primes come out like garbage.
Prefer something easier to print here

* Material: ASA or PLA.
* Layer height 0.16mm or lower
* Supports: tree
* Brim: Good idea but not required

Place the part with the back facing the plate like this
![Headphones](./guide-front-headphones-plate.png)

### Part 2
The fins can totally be printed in PLA. It will come out nice and it don't have thin parts that can easily break or crack. **Ideally use ASA.** You can use PETG, but petg is notoriously hard to sand and just a little moisture can make your primes come out like garbage.
Prefer something easier to print here

* Material: ASA or PLA.
* Layer height 0.16mm or lower
* Supports: tree
* Brim: Good idea but not required and they can affect the surface requiring more sanding

Once you load the STL file it will probally show in a wierd angle. 
![Fins](./guide-front-headphones-fin-part1.png)

Then will be needed to place it flat. Select in orca the tool to place it flat and choose one of the regions

![Fins](./place_flat.png)

Select some area on this region:
![Fins](./guide-front-headphones-fin-part2.png)

It should look something like this when placed

![Fins](./guide-front-headphones-fin-part3.png)

You can order to print like this, but lets be efficient. Create a copy of the model by selecting it and pressil ctrl-c and then ctrl-v.
Then right click on the new body and select mirror then mirror on the Y axis. If they are too close, move them apart. Should look like this:

![Fins](./guide-front-headphones-fin-part4.png)

When printing it should look like this:

![Fins](./guide-front-headphones-fin-part5.png)

If you see the plastic sagging at the tip of the fin, increase the support treshold angle or add a "support enforcer on the tip"

**BUT MOCK, CANT YOU PRINT IT WITH THE BACK ON THE PRINT BED INSTEAD????**
Yeah. You can! But the original mk16 fin has some soft curves that most printers will striggle to replicate with that orientation. Thats a great idea if you have a custom fin with a flat surface.
![Fins](./guide-front-headphones-fin-part6.png)

## Headphones Assembly guide

Now that you have all parts, now its a good time to sand it and trim all the support parts that got stuck. Doing it later will be harder.

![Headphones](./guide-front-headphones-assembly-part0.jpg)

Now grab two of the M4 screws, put them in two of the holes of one side until they come out on the other side. Use them to align the fins like this:

![Headphones](./guide-front-headphones-assembly-part1.png)

Retract the screws until the fin touches the headset. Screw the scres in to make sure they enter the fins holes. This is only to get them aligned.
Do this for both sides.

Once you get it right, its time to mix the two compound glue and make sure its well mixed.

![Headphones](./guide-front-headphones-assembly-part2.png)

Now spread between the screw holes on each of the fins. Avoid putting too much and avoid putting too close of the outer edge or the screw holes.
Putting too much or too close will cause the glue to get smushed and leak trough the outer side... Which you will have to clean it up. 
Glue on the screw hole will make you have a hard time taking the screws off and putting the heatset.

Save some glue for the magned in the top of the head.

![Headphones](./guide-front-headphones-assembly-part3.png)

Now put both of the fins in place and use the screws to keep them aligned. **Dont screw it all the way in**

![Headphones](./guide-front-headphones-assembly-part4.png)

If you notice the screws not beeing enough to get the fins completely straight and touching the whole sides, use the c-clamps to keep them close

![Headphones](./guide-front-headphones-assembly-part5.png)

Now use the leftover of the glue to fill a bit on the hole at the top of the head, and then place the magnet there.

![Headphones](./guide-front-headphones-assembly-part6.png)

![Headphones](./guide-front-headphones-assembly-part7.png)

Spread some glue on top of the magnet and if you have a few leftover magnets, put them on the other side to hold it in place

![Headphones](./guide-front-headphones-assembly-part8.png)

Now its a good idea do clean any bits of glue that came out of the edges. **Make sure you wash your hands very well too!**
Leave it still until the glue is completely cured.

__tic tac tic tac goes the clock__

Now grab your heatsets and your soldering iron with the heatset tip and warm ir up to 300ºC (If using PLA then 250).

![Headphones](./guide-front-headphones-assembly-part9.png)

Place the M3 (the long one), over the hole and put the soldering iron it, applying only a tiny amount of force. Wait a few seconds until you see the heatset going on the hole and melting the plastic around it.
The apply a little more pressure, but dont go too fast. We want it to go slowly while it melts the material around it. Make it go 2 to 3 mm deep. If you got 7mm long heatset, you can go as they get to the surface. If they're 5mm, then go that deep.

![Headphones](./heatset.gif)

Do this for all the holes without the screws. Then remove the screws and do it for them too.

![Headphones](./guide-front-headphones-assembly-part10.png)

Now to wrap it up. Get the the M3 3mm heat sets and put them on the holes in the back of the headset. Put them just enough so they align with the surface. Try to do it as straight as possible relative to the surface.

![Headphones](./guide-front-headphones-assembly-part11.png)

Once you finished, this part is complete!

![Headphones](./guide-front-headphones-assembly-part12.png)
