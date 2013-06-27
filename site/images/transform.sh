cp f36.ppm f36tmp.ppm 
convert f36mesh.png f36meshtmp.ppm 
mogrify -crop 850x950+350+15 -resize 320x240 f36meshtmp.ppm 
mogrify -crop 220x240+50+0 f36tmp.ppm 
convert -rotate 90 -append f36tmp.ppm f36meshtmp.ppm f36meshtmp.ppm 
convert -rotate -90 f36meshtmp.ppm result.png
rm *tmp.ppm

