import os, sys
from PIL import Image

imgs = os.listdir(os.getcwd())

#for infile in sys.argv[1:]:
for infile in imgs:
    f, e = os.path.splitext(infile)
    print("filename={}".format(f))
    print("extname={}".format(e))
    
    if (e == ".bmp") or (e == ".jpeg") or (e == ".jpg") or (e == ".png"):
	    outfile = f + "_5W.png"
	    img=Image.open(infile)
	    nWidth=img.size[0]
	    nHeight=img.size[1]
	    if nWidth > 2592:
	    	nWidth = 2592
	    	
	    if nHeight > 1944:
	    	nHeight = 1944
	    	
	    img_5w=Image.new(img.mode,(2592,1944),(255,255,255))
	    img_5w.paste(img,(0,0,nWidth,nHeight))
	    img_5w.save(outfile)
	    img_5w.close()
	    img.close()