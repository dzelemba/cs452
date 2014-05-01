l = [ ('D', 4), ('B', 6), ('C', 12), ('A', 4), ('B', 16), ('C', 10), ('B', 1), ('D', 14), ('E', 14), ('E', 9), ('D', 5), ('E', 6)]

for i in range(0, len(l)):
  fromc = str(l[i][0])
  fromd = str(l[i][1])
  toc = str(l[(i + 1) % len(l)][0])
  tod = str(l[(i + 1) % len(l)][1])

  print "printf(\"" + fromc + fromd + " -> " + toc + tod + ": %d\\n\", get_path_debug(track, '" + fromc + "', " + fromd + ", '" + toc + "', " + tod + ", path, &pathlen));"
