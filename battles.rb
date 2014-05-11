opponents = Hash.new
opponents["corey_abshire"] = "python opponents/corey_abshire/MyTronBot.py"
opponents["dmj111"] = "python opponents/dmj111/MyTronBot.py"
opponents["grogers"] = "./opponents/grogers/MyTronbot"
opponents["maxime81"] = "./opponents/maxime81/MyTronBot"
opponents["zerd"] = "./opponents/zerd/MyTronbot"
opponents["a1k0n"] = "./opponents/a1k0n/abot"

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
    Dir.foreach('battle_maps') do |map|
      next if map == "." or map == ".."
      next if rand(10) > 3
      cmd = "java -jar engine/Tron.jar maps/#{map} '#{trobo_exe}' '#{opp_exe}' "
      result = `#{cmd}`
      line = result.split[-1]
      if line == "Player One Wins!"
        wins[trobo_name] += 1.0
      elsif line == "Player Two Wins!"
        losses[trobo_name] += 1.0
      else
        wins[trobo_name] += 0.5
        losses[trobo_name] += 0.5
      end

      out_file = File.open("battles/#{trobo_name}_vs_#{opp_name}_on_#{map}", "w")
      out_file.puts(result)
      out_file.close
    end
  end
end

out_file = File.open("battles/summary.txt", "w")
wins.each do |trobo_name, wins|
  out_file.puts("Winning percentage for #{trobo_name} is #{100.0 * (wins/(wins + losses[trobo_name]))}")
end
out_file.close