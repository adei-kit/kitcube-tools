# use import CC2 to load the functions
# use reload(CC2) to reload after changes in the library
#
import Image


def names(dummy):

    # Description of the results
    
    return [ "sensor1", "sensor2", "sensor3"]



def mean(file, black = 10, white = 250): # return the mean value of the image

    print "Reading image: ", file 
    
    img = Image.open(file).convert("L")
    (x,y) = img.size
    
    n = 0;
    sum = 0;
    nwhite= 0;
    nblack = 0;
    data = img.load()
    for i in range (0, x):
	for j in range (0, y):                
		n += 1;
        	sum += data[i,j] 
   		if data[i,j] < black: nblack += 1
		if data[i,j] > white: nwhite += 1 

    result = [ float(sum)/n, float(nblack) / n, float(nwhite) / n ]

    #
    # print nice result plot
    #

    return result

