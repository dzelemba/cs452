#times = [ 522, 726, 1185, 714, 546, 757, 668, 689, 842, 688, 694, 762 ]
#speeds = [ 549, 516, 524, 525, 543, 535, 529, 545, 522, 545, 534, 522 ]

#times = [ 796, 656, 724, 836, 736, 704, 770, 556, 540, 1192, 554, 542 ]
#speeds = [ 508, 539, 519, 526, 509, 526, 516, 516, 509, 520, 496, 547 ]

times = [ 550, 740, 1240, 740, 564, 800, 678, 710, 838, 736, 702, 772 ]
speeds = [ 521, 506, 500, 506, 526, 506, 522, 529, 525, 509, 528, 512 ]

d = 0
for i in range(0, len(times)):
  d += times[i] * speeds[i]

tt = float(sum(times))

print d / tt
