TroBo
====================

A parallel tron bot for the [Tron AI Challenge](http://tron.aichallenge.org).

Our [15-418](http://15418.courses.cs.cmu.edu/spring2014/) final project.

Authors: Eric Wong, Jonathan Goldman

Instructions
---------------------

To run the bot, do

java -jar engine/Tron.jar maps/empty-room.txt ./MyTronBot "java -jar example_bots/Chaser.jar"

To see the output in a visualizer, do

1) java -jar engine/Tron.jar maps/empty-room.txt ./MyTronBot "java -jar example_bots/Chaser.jar" > match.out

2) ruby visualizer/format_output.rb match.out formatted_match.out

3) Open visualizer/visualizer.html in a web browser, and drag formmated_match.out into the box.