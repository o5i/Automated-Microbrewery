import RPi.GPIO as GPIO, time, sys

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM) 
GPIO.setup(17, GPIO.OUT) 
p = GPIO.PWM(17, 1800)   
p.start(50)
p.ChangeDutyCycle(50)
for i in range(0, 450):
	p.ChangeFrequency(i+900)
	time.sleep(0.003)
# 	p.ChangeFrequency(1400)
# 	time.sleep(0.11)
p.ChangeDutyCycle(0)
GPIO.cleanup()
sys.exit(0)
