# rdt-lab

## Running the code
The code must be run on the hydra machine and can be compiled and executed with 
the command below.

### Commands
`gcc -w -o main main.c && ./main`

## Notes
When prompted for `Enter average time between messages from sender's layer5 [ > 0.0]`, 
setting it to small values (like 0.1) causes the program to run for a very short time.
Set it to a higher number for the program to run longer and allow more messages be
delivered.  

For reference:
![image](https://user-images.githubusercontent.com/25562345/229226441-b94b2d03-791e-4048-9d44-259688de0d61.png)
![image](https://user-images.githubusercontent.com/25562345/229980214-b09a5353-93fa-4cfb-adf4-97d82d742d8c.png)