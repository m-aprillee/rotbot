from flask import Flask
from flask import request
app = Flask(__name__)

temp = 0
humidity = 0
light = 0
state = ""
text_color = ""

@app.route("/")
def hello():
    global temp, humidity, light, state, text_color
    if request.args.get("temp"):
        temp = request.args.get("temp")
    if request.args.get("humidity"):
        humidity = request.args.get("humidity")
    if request.args.get("light"):
        light = request.args.get("light")
    if request.args.get("state"):
        state = request.args.get("state")
        if state == "GREEN":
            text_color = 'green'
        elif state == "YELLOW":
            text_color = 'yellow'
        else:
            text_color = 'red'
    
    header = "<h1 style='font-family:courier; text-align:left;'>RotBot Results</h1>"
    state_styled = "<h2 style='font-family:courier; text-align:left; color:" + text_color + "; font-size: 200%;'> " + state + "</h2>"
    temp_styled =  "<p style='font-family:courier; text-align:left; ;'>Temperature: " + str(temp) + "°C</p>"
    humidity_styled = "<p style='font-family:courier; text-align:left;'>Humidity: " + str(humidity) + "</p>"
    light_styled = "<p style='font-family:courier; text-align:left;'>Light: " + str(light) + "</p>"
    return "<body style='margin: 50px 50px 50px 50px; color: white; background-color: black; font-size: 200%;'>" + header + state_styled + temp_styled + humidity_styled + light_styled + "</body>"