#    ESP8266 OLED Weather Display Message Board
#    Copyright (C) 2023  Sam Warr
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

import os
import string
from secretfile import topic
from secretfile import server

print("publisher.py Copyright (C) 2023  Sam Warr")
print("This program comes with ABSOLUTELY NO WARRANTY")
print("This is free software, and you are welcome to redistribute it")
print("under certain conditions")
print()

# The topic of the MQTT server to publish on
print("Publishing on topic [" + topic + "]")
while True:
# Prompt the user for a message
    message = input("Message: ")
# Define the bash command
    command = "mosquitto_pub -h " + server + " -t \"" + topic + "\" -m \"" + message + "\""
# Execute the command
    os.system(command)
