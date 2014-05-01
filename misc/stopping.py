a = -9.64859

def stop_time(v):
  return (-1 * v) / a

def stop_dist(v, t):
  return v * t + a * t * t

velo = 705.592
time = stop_time(velo)
print 'time ticks: ', time, ' distance um: ', stop_dist(velo, time)

velo = 5311.245
time = stop_time(velo)
print 'time ticks: ', time, ' distance um: ', stop_dist(velo, time)
