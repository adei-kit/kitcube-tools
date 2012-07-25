# Use import CC2 to load the functions
# use reload(CC2) to reload after changes in the library
#
# TODO: Calculate the brightness considering the optical parameters !!!
#   


import Image
import ExifTags

def names(dummy):

    # Description of the results
    
    return [ "sensor1", "sensor2", "sensor3", "sensor4", "sensor5"]



def mean(file, black = 10, white = 245): # return the mean value of the image

    #print "Reading image: ", file 
    try:    
       cimg = Image.open(file)

       # Get image properies (exposure, apperture, ...)
       exif = {} 
       info = cimg._getexif()

       #for tag, value in info.items():
       #   decoded = TAGS.get(tag, tag)
       #   exif[decoded] = value
       #
       #print exif 
    except:
       return [0, 0, 0, 0, 0]


    result = {'errors': None}
    try:
        t = info[33434]    # shutter time
        exposure = t[1]/t[0]
        result['exposure'] = '1/%d sec.' % (t[1]/t[0])
    except:
        result['exposure'] = None
        exposure = 0

    try:
        d = info[33437]    # diafragma
        apperture = float(d[0])/d[1]
        result['shutter'] = 'F/%.1f' % (float(d[0])/d[1])
    except:
        result['shutter'] = None
        apperture = 0

    try:
        focus = info[37386]
        focus = focus[0]/focus[1]
        result['focus'] = '%d mm.' % (focus[0]/focus[1])
    except:
        result['focus'] = None
        focus = 0

    #print result
  
   
    
    # Convert to greyscale and resize image 
    try:
       limg = cimg.convert("L")
       (x,y) = limg.size
       #print "Image size is " + str(x) + 'x' + str (y) 

       centercrop = 0.65
       height = int ( centercrop * y ) 
       width = height * 4 / 3
       xmin = (x-width)/2
       xmax = xmin + width
       ymin = (y-height)/2
       ymax = ymin + height

       center = (xmin,ymin,xmax,ymax)
       limg= limg.crop(center)

       img = limg.resize((480,320))
       #img.show()

       # Calculate pixel statisitcs
       n = 0;
       sum = 0;
       nwhite= 0;
       nblack = 0;
       ngrey = 0;
       data = img.load()
       (x,y) = img.size
       for i in range (0, x):
	 for j in range (0, y):                
            n += 1;
            if data[i,j] < black: 
            		nblack += 1
            elif data[i,j] > white: 
            		nwhite += 1 
            else:
            		ngrey += 1
            		sum += data[i,j] 

       if (ngrey < n/10) and (nblack > 0.8 * n):
          sum = 0;
          ngrey = 1;

       if (ngrey < n/10) and (nwhite > 0.25 * n):
	  sum = 255;
	  ngrey = 1;
     
       result = [ exposure, apperture, float(sum)/ngrey, 100.0 * float(nblack) / n, 100.0 * float(nwhite) / n]

    except: 
       
       result = [ exposure, apperture, 0, 0, 0]    
           
           
    #
    # print nice result plot
    #

    return result

