# EECS149-Group-project
This repository is for embeded system final project  
## Project:
Tri-Robot Cooperation 

## Project goal:
We set the Kobuki performing route to be:
Randomly place → Automatically forming a triangle (three Kobuki’s move without following the leader Kobuki )→Followers and the leader Kobuki do other commands of the instructor.
![image](https://github.com/CZhaoYoung/EECS149-Group-project/blob/master/architecture_drawings/task%20route.png)

We decided to use 8 ultra wideband transceivers, including 4 anchors, 3 tags and 1 listener to measure position of each Kobuki(i.e. its x,y,z coordinates).



## Achitecture 
![image](https://github.com/CZhaoYoung/EECS149-Group-project/blob/master/architecture_drawings/Untitled%20Diagram%20(4).png)

(1) BLE communication:<br>
Actor manipulates computer by pressing the start button or moving button to control the kobuki.  Eventually, the command is sent to the leader of kobukis from the computer by BLE,  and then the leader kobuki will advertise the commands by BLE to other kobukis to realize communication with each other. <br>
(2) UWB communication: <br>
We will utilize the UWB device (DWM 1001C) to form the distance measurement system to detect the distance between each kobukis. We apply the “ 4 anchors + 3 Tags + 1 Listener” system configuration option in our project, by configuring one of the devices as a listener device, the data can be captured to a PC directly. <br>
(3) Coordinate System Establishment:<br>
As we have the data from the listener (DWM1001C), we can build a coordinate system by setting the location of the leader kobuki as an origin. The initial orientation of the kobuki can be obtained by pressing the start button to move the kobuki and then to get the vector, which can be regarded as the initial orientation of the kobuki.<br>

## State Machine 
![image](https://github.com/CZhaoYoung/EECS149-Group-project/blob/master/architecture_drawings/statemachine.png)

## Identification of major risks <br>
(1) We need to consider the drifts of each kobukis, which will be a tricky task in the coming weeks. It is challenging  if the kobuki do not follow the route we set. <br>
(2) The tag (DWM1001C)  that is being tracked on the tablet must be in Bluetooth range of the tablet, if it is not on the range, it may not work well. <br>
(3) We need to consider the time delay of BLE connections between the devices.<br>


## Team members:  
Jieming Fan  
Zhaoyang Chen  
Bingjie Zhu  
Manqin Zhong  

