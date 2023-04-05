# rdt-lab
![image](https://user-images.githubusercontent.com/25562345/229226441-b94b2d03-791e-4048-9d44-259688de0d61.png)
<img width="1122" alt="image" src="https://user-images.githubusercontent.com/25562345/229980214-b09a5353-93fa-4cfb-adf4-97d82d742d8c.png">

## Running the code
So far I have only been able to run the code on the hydra machines. You can run 
it on your personal machine if you have an older version of a C compiler.

When prompted for `Enter average time between messages from sender's layer5 [ > 0.0]`  
Setting it to small values, like 0.1, causes `B_Input` to not be called? Setting it to
10+ seems to work.

### Commands
`gcc -w -o main main.c`
`./main`
