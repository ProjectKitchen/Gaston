# Gaston

![Break Off](img/Gaston_detail.jpg)

Gaston is a cocktail serving robot on wheels which is delighted to offer safe transportation of your beverage of choice - right to your seat.
 
## Features

* Follows a line with several stops marked by RFID tags
* Plays Music from a SD-card
* Accepts orders of different kinds of cocktails via selection buttons
* Communicates with delivery station via infrared signals - cocktail recipes can be changed easily 
* RGB LED strip for outstanding light-effects and cocktail indication
* RFID-Sensor for determining table position and base station 
* base station with up to 4 MOSFET-controlled pumps for ingredients
* Rechargeable battery for hours of operation


## Operation:
Gaston patrols between different stations along the line (where guests can place orders). He will wait for about ten seconds at each station, and then proceed to the next. At the last station, Gaston makes a U-turn and travels back to the first station. 

To choose a desired cocktail, place an empty glass (!) in the glass holder. 
Then press the white button to choose a desired cocktail. The robot voice offers suggestions for specific mixtures. For confirmation, press the green button. Gastón will then head towards the delivery station, get your drink and (hopefully) come back.

## Startup:
When Gaston is switched on, the sensor settings are calibrated for the current track surface. 
Place the robot on the surface, a few centimeters right of the line. Press the white button. 
A threshold is calculated and Gastón will start searching the line (driving to the left until the darker line material is found).


## Track buildup and material selection
A line connects all the stations. The line is usually painted with black (matt) color or made from adhesive tape. 
The line should reflect less IR-light than the driving track surface. Non-reflecting black tape on a bright surface works best. 
The RFID tags (flat cards) are placed along the line - the line is drawn on top of the RFID cards.
The unique IDs of the tags must be given in the code-array (source file: https://github.com/ProjectKitchen/Gaston/blob/master/src/Gaston/drive.c#L26 ) 
The base station is represented by the first code in the array, then follow the table stations. 


## More infos about build and programming:
see: (doc/Documentation.doc)

