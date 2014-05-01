import math

s = 0
s2 = 0
d = 0

for t in range(0, 500):
  j = max(-6 * t * t + 3463 * t - 279770, 0) # 1000000 * cm /s === 10 * nm / tick
  k = max(-2 * t * t * t + 1731 * t * t - 279770 * t, 0)
  #print t, ':+', j
  s += j
  d += s
  s2 += j / 10
  v = k

  print t, 'velo:', s, 'dx:', d
  if (s > 53300000):
    break

print s
print s2
print v
