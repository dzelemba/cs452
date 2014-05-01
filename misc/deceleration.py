import pprint
import math

MM_TO_UM = 1000

# um per tick
velo = [ x for x in [ 89.622, 705.592, 1227.363, 1730.769, 2193.155, 2712.882, 5192.361 ] ]
# um
dist = [ x * 1000 for x in [ 30.0, 83.6, 165.6, 232.2, 296.3, 364.5, 700.0 ] ]

def tee(v):
  return v / 5192.361 * 400

def quad1(a, b, c):
  if (b ** 2 - 4 * a * c < 0):
    return 0
  return (-b + math.sqrt(b ** 2 - 4 * a * c)) / (2 * a)

def quad2(a, b, c):
  if (b ** 2 - 4 * a * c < 0):
    return 0
  return (-b - math.sqrt(b ** 2 - 4 * a * c)) / (2 * a)

def errat(j, k = 0, l = 0, dbg = False):
  err = 0
  for i in range(0, len(velo)):
    v = velo[i]
    #t = tee(v)
    t = (v / j) * -1

    #q1 = quad1(j / 2.0, k, v)
    #q2 = quad2(j / 2.0, k, v)
    #if (q1 <= 0 and q2 <= 0):
      #return float('inf')
    #elif (q1 <= 0):
      ##opt = max(opt, q2)
      #t = q2
    #elif (q2 <= 0):
      ##opt = max(opt, q1)
      #t = q1
    #else:
      #t = min(q1, q2)

    # We've found t, now find error in this model (t, j, k)
    #dt = v * t + (j * t ** 4) / 12 + (k * t ** 3) / 6 + (l * t ** 2) / 2
    dt = v * t + (j * t) / 2;
    e = dist[i] - dt

    if (dbg):
      print 't: ', t
      print dist[i], ' vs ', dt

      #print 'd: ', dist[i]
      #print 'acc: ', v * t + j * t ** 3 + k * t ** 2
      #print 't,e: ', t, e
    err += e*e

  return err

#a = -0.00955873
#a = -0.01730787

#def ale():
  #for i in range(0, len(velo)):
    #t = -(velo[i] / a)
    ##print 't: ', t
    ##print 'velo * t: ', velo[i] * t
    ##print 'att: ', a * t * t / 2
    #print dist[i], ' vs ', velo[i]*t + a*t*t/2


opt = 0
def ale():
  minj = 0
  mink = 0
  minl = 0
  minerr = float('inf')

  #for j in range(-100, 100):
  for J in range(-1000, 0):
    #j = J / 100000.0
    #print 'j == ', j
    j = J
    err = errat(j)

    if (err < minerr):
      #print 'J: ', j, ' K: ', k, ' L: ', l, 'Err:', err
      print 'J: ', j, 'Err:', err
      minerr = err
      minj = j
      #mink = k
      #minl = l

    #for K in range(-100, 100):
      #k = K / 10.0
      #for l in range(-100, 100):
        #err = errat(j, k, l, False)

  errat(minj, 0, 0, True)


ale()
#errat(-1, 81, True)
#errat(0.00087, -0.4, 13, True)
