opponents = Hash.new
opponents["corey_abshire"] = "python opponents/corey_abshire/MyTronBot.py"
opponents["dmj111"] = "python opponents/dmj111/MyTronBot.py"
opponents["grogers"] = "./opponents/grogers/MyTronbot"
opponents["maxime81"] = "./Opponents/maxime81/MyTronBot"
opponents["zerd"] = "./Opponents/zerd/MyTronbot"
opponents["a1k0n"] = "./Opponents/a1k0n/abot"

trobo = Hash.new
trobo["minimax"] = "./MyTronBot"
trobo["ab"] = "./MyTronBot --ab"
trobo["minimax_parallel"] = "./MyTronBot --p"
trobo["ab_parallel"] = "./MyTronBot --ab --p"
trobo["ab_parallel_abort"] = "./MyTronBot --ab --p --abort"

wins = Hash.new
wins.default = 0.0

losses = Hash.new
losses.default = 0.0

opponents.each do |opp_name, opp_exe|
  trobo.each do |trobo_name, trobo_exe|
    Dir.foreach('maps') do |map|
      next if map == "." or map == ".."
        cmd = "java -jar engine/Tron.jar maps/#{map} '#{trobo_exe}' '#{opp_exe}' "
        result = `#{cmd}`
        if result.lines.last == "Player One Wins!"
          wins[trobo_name] += 1.0
        elsif result.lines.last == "Player Two Wins!"
          losses[trobo_name] += 1.0
        elsif result.lines.last == "Players collided. Draw!"
          wins[trobo_name] += 0.5
          losses[trobo_name] += 0.5
        end

        out_file = File.new("battles/#{trobo_name}_vs_#{opp_name}_on_#{map}")
        out_file.puts(result)
        out_file.close

  end
end

out_file = File.new("battles/summary.txt")
wins.each do |trobo_name, wins|
  out_file.puts("Winning percentage for #{trobo_name} is #{100.0 * (wins/(wins + losses[trobo_name]))}")
end