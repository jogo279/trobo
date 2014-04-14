infile = File.open(ARGV[0], "r")
outfile = File.new(ARGV[1], "w")

outstring = "+OK|15 15|"

dimensions = infile.gets
width = dimensions.split[0].to_i
height = dimensions.split[1].to_i

i = 0
while i < height do
  line = infile.gets
  if line.include? "1"
    p1x = line.index("1")
    p1y = i
  end
  if line.include? "2"
    p2x = line.index("2")
    p2y = i
  end
  outstring += line
  i += 1
end

outstring += "|TroBo|Opponent|"

while infile.gets == dimensions do
  i = 0
  while i < height do
    line = infile.gets
    if line.include? "1"
      new_p1x = line.index("1")
      new_p1y = i
    end
    if line.include? "2"
      new_p2x = line.index("2")
      new_p2y = i
    end
    i += 1
  end
  if new_p1x > p1x
    outstring += "E"
  elsif new_p1x < p1x
    outstring += "W"
  elsif new_p1y > p1y
    outstring += "S"
  else
    outstring += "N"
  end
  if new_p2x > p2x
    outstring += "E"
  elsif new_p2x < p2x
    outstring += "W"
  elsif new_p2y > p2y
    outstring += "S"
  else
    outstring += "N"
  end
  p1x = new_p1x
  p1y = new_p1y
  p2x = new_p2x
  p2y = new_p2y
end

outstring += "|+OK"
outfile.write outstring
infile.close
outfile.close