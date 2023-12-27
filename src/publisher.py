#write a program that prompts for a message then executes a bash command with one of the parameters being the message
#the bash command should be a parameter to the python program
#the message should be a parameter to the bash command
import os
import string

while True:
# Prompt the user for a message
    message = input("Message: ")

# Define the bash command
    command = "mosquitto_pub -h test.mosquitto.org -t \"samwarr2001\" -m \"" + message + "\""
# Execute the command
    os.system(command)
