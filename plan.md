## Data Type:
 FLOAT    4 bytes 
 INT8     1 bytes 
 INT16    2 bytes 
 INT32    4 bytes 
 DOUBLE64   8 bytes 
 XYTHETA       //look yourself bitch, same as that in rtos GAY codes 
 ENUMTYPE      


## Init Stages:

|Computer|<----------------------->|Jetson|
|----|-----|----|
|  Direction | Meaning | Size| 
|  <<<    | Init byte     | 1 byte     |
|>>>| ready byte| 1 byte
|  |Init Loop Start||
|<<< |init start byte| 1 byte|
|<<<|id|int8 1 byte for now|
|<<<|strlen of variable name| int8/1 byte|
|<<<|name|chars n byte|
|<<<|data type|1 byte|
|  |Back to Init Loop Start, otherwise if finish go below||
|<<<|end byte|1 byte|
|>>>|end confirmed byte|1 byte|


***Remarks***
       if type is ENUMTYPE[which is 1 byte],
       <<<       strlen of enum Name                 
       <<<       enum Name                      
       <<<       strlen of enum Element name                 
       <<<       enum Element name                              
       <<<       strlen of enum Element name                                 
       <<<       enum Element name               
       <<<       '/'[end enum byte]
       then go back to init loop start, to continue sending next "object" id

      
## Transfer Stage:
|Computer |<------------------------------------->|Jetson|
|---|---|---|
|>>>|start byte|1 byte|
|<<<|start2 byte| 1 byte|
||"Transfer Loop"||
|<<<|Id|1 byte|
|<<<|time|4 byte(float)|
|<<<|data byte|n byte calculated during init|
||  goto "Transfer Loop" if still have variables/object to send      |  |

     
if you want byte:
      >>>       pause you bitch byte                    1 byte  
      
