import os, sys
from PIL import Image

imgs = os.listdir(os.getcwd())

#for infile in sys.argv[1:]:
for infile in imgs:
    f, e = os.path.splitext(infile)
    print("filename={}".format(f))
    print("extname={}".format(e))
    
    if (e == ".bmp") or (e == ".jpeg") or (e == ".jpg") or (e == ".png"):
	    outfile = f + "_o.png"
	    img=Image.open(infile)
	    nWidth=img.size[0]
	    nHeight=img.size[1]
	    if (nWidth%2) != 0:
	    	nWidth += 1
	    	
	    if (nHeight%2) != 0:
	    	nHeight += 1
	    	
	    img_5w=Image.new(img.mode,(nWidth,nHeight),(255,255,255))
	    img_5w.paste(img,(0,0,img.size[0],img.size[1]))
	    img_5w.save(outfile)
	    img_5w.close()
	    img.close()