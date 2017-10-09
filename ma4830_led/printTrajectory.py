import math
v=100
h=200
theta = 60*3.14/180
# deltaY = 0.2
# deltaX = 0.03
g=9.8



def Downfunc(y):
    sqrtEq = 1 +  ((2*g*(h - y))/(((math.sin(theta)**2)*v**2))) 
    d = (v**2/(2*g))  *  (1 +  math.sqrt(abs(sqrtEq)) )  *  math.sin(2*theta)
    return d

def Upfunc(y):
    sqrtEq = 1 +  ((2*g*(h-y))/(((math.sin(theta)**2)*v**2))) 
    d = (v**2/(2*g))  *  (1 -  math.sqrt(abs(sqrtEq)) )  *  math.sin(2*theta)
    return d

def PlotAboveH(y, deltaX):
    while (y >= h and y <= yMax ):
        upX = int(math.ceil(Upfunc(y)/deltaX))
        # print("Up:: y>> {}, x>> {}, {}".format(y, Upfunc(y), upX))

        downX = int(math.ceil(Downfunc(y)/deltaX))
        # print("Down:: y>> {}, x>> {}, {}".format(y, Downfunc(y), downX) )

        #indent in front y axis:
        string = str(round( y ,1)) + " "*( Yindent-len(str(round(y,1))) )  +"|"

        for idx in range(180):
            if idx == upX or idx == downX:
                string = string + "x"
            else:
                string = string + " "
        print(string)
        y = y- deltaY
    return y

def PlotBelowH(y, deltaX):
    while (y >= 0 and y <= h ):
        downX = int(math.ceil(Downfunc(y)/deltaX))
        # print("Down:: y>> {}, x>> {}, {}".format(y, Downfunc(y), downX) )
        string = str(round( y ,1)) + " "*( Yindent-len(str(round(y,1))) )  +"|"
        for idx in range(180):
            if idx == downX:
                string = string + "x"
            else:
                string = string + " "
        print(string)    
        y = y- deltaY
    return y

def PlotXaxis(Yindent, deltaX):
    print("_" * 180)
    # x=0
    # while  x <= d :
    # downX = int(math.floor(Downfunc(y)*10))
    interval = 10
    string = "(0,0)" + " "*( Yindent-4 )
    idx = 1
    #going index to index
    while(idx<math.ceil(dMax/deltaX)):
        if idx%interval == 0:
            # if idx< 10
            Xindent = str(round(idx*deltaX,3)) 
            string = string + Xindent 
            idx = idx + len(Xindent)
            # if idx>math.ceil(dMax/deltaX):
            #     break
        else:
            string = string + " "
            idx = idx + 1
    print(string)
    return 0

def XYscalling(yMax,dMax):
    #rescale
    #coenfiicient is length and width of screen
    deltaY = round(yMax/50,2)
    deltaX = round(dMax/150,3)
    #coenficient is delta how many deltaX unit to deltaY unit
    if deltaY < 2*deltaX: 
        deltaY = 2*deltaX
    else:
        deltaX = deltaY/2
    print("delta xy is ({} , {})".format(deltaX, deltaY))
    return (deltaX,deltaY)


dMax = Downfunc(0)
yMax = h + (v**2)*(math.sin(theta)**2)/(2*g)

# print("from Up: this is max y>> {}, x>> {}".format(yMax, Upfunc(yMax)))
# print("from Down: this is max y>> {}, x>> {}\n".format(yMax, Downfunc(yMax)))
print("\tTRAJECTORY MOTION OF: v = {}, h = {}, theta = {}".format(v, h, theta))
print("y Max is {}".format(yMax))

Yindent = len(str(round(yMax,1)))

deltaXY = XYscalling(yMax,dMax)
deltaX = deltaXY[0]
deltaY = deltaXY[1]

y=yMax
y = PlotAboveH(y, deltaX)
y = PlotBelowH(y, deltaX)
PlotXaxis(Yindent, deltaX)

print("D Max is {}".format(dMax))



# y=h
# while(True):
#     print("Up:: y>> {}, x>> {}".format(y, Upfunc(y)))
#     y = y+ 0.1
#     if y> yMax:
#         y = y- 0.1
#         break

# while (True):
#     print("Down:: y>> {}, x>> {}".format(y, Downfunc(y)) )
#     y = y- 0.1
#     if y< 0:
#         break



# y = h + math.sin(theta)*v*t - 2*g*t**2
